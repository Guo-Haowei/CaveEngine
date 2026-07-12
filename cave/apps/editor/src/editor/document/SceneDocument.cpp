#include "SceneDocument.h"

#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneRuntime.h"
#include "cave/runtime/scene/SceneTickContext.h"

#include "engine/private/runtime/assets/PrefabAsset.h"
#include "engine/private/runtime/assets/SceneAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

SceneDocument::SceneDocument(EngineServices& services, const Guid& guid)
    : DocumentBase(services, guid) {

    auto scene = createPreviewScene();

    scene->begin(MakeOwner<SceneRuntime>(
        SceneTickDomain::Editor,
        m_engine_services,
        *scene));

    if (auto handle_opt = m_asset_reg.findByGuid(guid)) {
        const AssetMetaData* meta = handle_opt.unwrap_unchecked().meta();
        if (DEV_VERIFY(meta)) {
            DEV_ASSERT(meta->type == AssetType::Scene || meta->type == AssetType::Prefab);
            m_preview_scene = m_scene_reg.registerScene({ SceneSource::Editor, meta->name },
                std::move(scene));
        }
    }
}

SceneDocument::~SceneDocument() {
    if (Scene* scene = m_scene_reg.resolve(m_preview_scene)) {
        scene->end();
    }
}

bool SceneDocument::save() {
    if (auto asset = dynamic_cast<SceneContainer*>(m_handle.get())) {
        Scene* tmp = m_scene_reg.resolve(m_preview_scene);
        asset->sceneMut().copy(*tmp);
        return DocumentBase::save();
    }
    return false;
}

Owner<Scene> SceneDocument::createPreviewScene() const {
    if (auto asset = dynamic_cast<SceneContainer*>(m_handle.get())) {
        auto scene_copy = MakeOwner<Scene>();
        scene_copy->copy(asset->scene());
        return scene_copy;
    }

    return nullptr;
}

}  // namespace cave
