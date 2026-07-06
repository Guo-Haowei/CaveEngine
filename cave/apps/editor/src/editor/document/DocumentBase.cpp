#include "DocumentBase.h"

#include "cave/runtime/framework/EngineServices.h"

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

    handle_ = m_asset_reg.findByGuid(guid).unwrap();
    asset_ = handle_.wait();
}

bool DocumentBase::apply(std::unique_ptr<IEditCmd> cmd, uint32_t coalesce) {
    if (!cmd) return false;

    if (!m_undo.empty() /*&& coalesce != 0 && last_coalesce_ == coalesce*/) {
        IEditCmd* last = m_undo.back().get();
        if (last && last->canCoalesceWith(cmd.get())) {
            cmd->apply(*this);
            last->coalesceFrom(std::move(cmd));
            m_redo.clear();
            touchDirtyAfterEdit();
            return true;
        }
    }

    cmd->apply(*this);

    m_undo.push_back(std::move(cmd));
    m_redo.clear();

    m_last_coalesce = coalesce;
    touchDirtyAfterEdit();
    trimUndoIfNeeded();

    return true;
}

bool DocumentBase::undo() {
    if (m_undo.empty()) return false;
    auto cmd = std::move(m_undo.back());
    m_undo.pop_back();

    cmd->undo(*this);
    m_redo.push_back(std::move(cmd));

    m_last_coalesce = 0;
    recomputeDirtyAfterHistoryMove();
    return true;
}

bool DocumentBase::redo() {
    if (m_redo.empty()) return false;
    auto cmd = std::move(m_redo.back());
    m_redo.pop_back();

    cmd->apply(*this);
    m_undo.push_back(std::move(cmd));

    m_last_coalesce = 0;
    recomputeDirtyAfterHistoryMove();
    return true;
}

void DocumentBase::undoLabels(std::vector<std::string>& out, int max_items) const {
    out.clear();
    int count = 0;
    for (auto it = m_undo.rbegin(); it != m_undo.rend() && count < max_items; ++it, ++count) {
        out.emplace_back((*it)->label());
    }
}

void DocumentBase::redoLabels(std::vector<std::string>& out, int max_items) const {
    out.clear();
    int count = 0;
    for (auto it = m_redo.rbegin(); it != m_redo.rend() && count < max_items; ++it, ++count) {
        out.emplace_back((*it)->label());
    }
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

std::unique_ptr<Scene> DocumentBase::createPreviewScene() const {
    return nullptr;
}

void DocumentBase::reloadPreviewScene() {
    auto new_scene = createPreviewScene();
    if (new_scene == nullptr) {
        return;
    }

    SceneTickContext ctx = {
        .domain = SceneTickDomain::Editor,
        .dt = 0.0f,
        .scene_ctx = {
            .scene = *new_scene,
            .query = SceneQuery(*new_scene),
            .engine_services = m_engine_services,
        },
    };

    new_scene->begin(ctx);

    Scene* old_scene = m_scene_reg.resolve(m_preview_scene);
    if (DEV_VERIFY(old_scene)) {
        old_scene->end();
    }

    LOG_INFO(LogChannel::Asset, "reload scene {}", m_preview_scene.toString());
    m_scene_reg.replaceScene(m_preview_scene, std::move(new_scene));
}

}  // namespace cave
