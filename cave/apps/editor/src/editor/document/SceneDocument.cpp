#include "SceneDocument.h"

#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneRuntime.h"
#include "cave/runtime/scene/SceneTickContext.h"

#include "engine/private/runtime/assets/PrefabAsset.h"
#include "engine/private/runtime/assets/SceneAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"

namespace cave {

SceneDocument::SceneDocument(EngineServices& services, const Guid& guid)
    : DocumentBase(services, guid) {

    auto scene = createPreviewScene();

    scene->alwaysRun(MakeOwner<SceneRuntime>(
        SceneTickDomain::Editor,
        m_engine_services,
        *scene,
        ViewId{}));

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

bool SceneDocument::changeProperty(const PropertyTarget& target,
                                   const uint8_t* data,
                                   size_t data_size) {
    SceneId scene_id = m_preview_scene;
    if (!scene_id.valid()) return false;
    Scene* scene = m_engine_services.sceneRegistry().resolve(scene_id);
    if (!scene) return false;

    const auto* component = std::get_if<ComponentPropertyTarget>(&target);
    if (!component) {
        return false;
    }

    SceneCommandExecutor executor(*scene);
    bool res = executor.changeProperty(component->entity,
                                       component->cid,
                                       component->pid,
                                       data,
                                       (uint32_t)data_size);
    return res;
}

}  // namespace cave
