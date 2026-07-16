#include "DocumentBase.h"

#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneRuntime.h"

#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

#include "editor/EditorAssetManager.h"

#define DEBUG_DOC IN_USE
#if USING(DEBUG_DOC)
#define DEBUG_DOC_LOG(...) LOG_TRACE(__VA_ARGS__)
#else
#define DEBUG_DOC_LOG(...) (void)0
#endif

namespace cave {

DocumentBase::DocumentBase(EngineServices& services, const Guid& guid)
    : m_engine_services(services)
    , m_asset_reg(services.assetRegistry())
    , m_asset_mgr(static_cast<EditorAssetManager&>(services.assetManager()))
    , m_scene_reg(services.sceneRegistry())
    , m_guid(guid) {

    m_handle = m_asset_reg.findByGuid(guid).unwrap();
    m_asset = m_handle.wait();
}

DocumentBase::~DocumentBase() = default;

bool DocumentBase::apply(Owner<IEditCmd> cmd, uint32_t coalesce) {
    if (!cmd) {
        return false;
    }

    const bool can_coalesce =
        coalesce != 0 &&
        coalesce == m_last_coalesce &&
        !m_undo.empty() &&
        m_undo.back().cmd->canCoalesceWith(cmd.get());

    if (can_coalesce) {
        cmd->apply(*this);

        EditRecord& last = m_undo.back();
        last.cmd->coalesceFrom(std::move(cmd));

        // Although this remains one undo operation, it is a new document state.
        last.after_state = m_next_state++;
        m_current_state = last.after_state;

        m_redo.clear();
        touchDirtyAfterEdit();
        return true;
    }

    const EditStateId before_state = m_current_state;
    const EditStateId after_state = m_next_state++;

    cmd->apply(*this);

    m_undo.push_back(EditRecord{
        .cmd = std::move(cmd),
        .before_state = before_state,
        .after_state = after_state,
    });

    m_current_state = after_state;
    m_redo.clear();

    m_last_coalesce = coalesce;

    touchDirtyAfterEdit();
    trimUndoIfNeeded();
    return true;
}

bool DocumentBase::undo() {
    if (m_undo.empty()) {
        return false;
    }

    EditRecord record = std::move(m_undo.back());
    m_undo.pop_back();

    record.cmd->undo(*this);
    m_current_state = record.before_state;

    m_redo.push_back(std::move(record));

    // Prevent the next edit from accidentally joining an old interaction.
    m_last_coalesce = 0;

    touchDirtyAfterEdit();
    return true;
}

bool DocumentBase::redo() {
    if (m_redo.empty()) {
        return false;
    }

    EditRecord record = std::move(m_redo.back());
    m_redo.pop_back();

    record.cmd->apply(*this);
    m_current_state = record.after_state;

    m_undo.push_back(std::move(record));

    m_last_coalesce = 0;

    touchDirtyAfterEdit();
    return true;
}

void DocumentBase::trimUndoIfNeeded() {
    if (m_undo_limit == 0) return;
    while (m_undo.size() > m_undo_limit) {
        // If we drop history older than the save marker,
        // we must shift the marker accordingly to preserve meaning.
        m_undo.pop_front();
        if (m_saved_undo_size > 0) {
            m_saved_undo_size -= 1;
        }
    }
}

bool DocumentBase::save() {
    bool ok = m_asset_reg.saveAsset(m_guid);
    if (ok) {
        m_saved_state = m_current_state;

        m_asset_mgr.onAssetSaved({
            .reason = AssetChangeReason::Saved,
            .revision = m_asset_reg.revision(m_guid),
            .guid = m_guid,
        });
    }
    return ok;
}

bool DocumentBase::saveAs(std::string_view) {
    return false;
}

Owner<Scene> DocumentBase::createPreviewScene() const {
    return nullptr;
}

void DocumentBase::reloadPreviewScene() {
    auto new_scene = createPreviewScene();
    if (new_scene == nullptr) {
        return;
    }

    new_scene->alwaysRun(MakeOwner<SceneRuntime>(
        SceneTickDomain::Editor,
        m_engine_services,
        *new_scene,
        ViewId{}));

    Scene* old_scene = m_scene_reg.resolve(m_preview_scene);
    if (DEV_VERIFY(old_scene)) {
        old_scene->end();
    }

    m_scene_reg.replaceScene(m_preview_scene, std::move(new_scene));
}

}  // namespace cave
