#include "SceneDocument.h"

#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneTickContext.h"

#include "engine/private/runtime/assets/PrefabAsset.h"
#include "engine/private/runtime/assets/SceneAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

SceneDocument::SceneDocument(EngineServices& services, const Guid& guid)
    : DocumentBase(services, guid) {

    auto scene = createPreviewScene();

    SceneContext ctx = {
        .scene = *scene,
        .query = SceneQuery(*scene),
        .services = services,
    };
    scene->begin({
        .domain = SceneTickDomain::Editor,
        .dt = 0.0f,
        .scene_ctx = ctx,
    });

    if (auto handle_opt = m_asset_reg.findByGuid(guid)) {
        const AssetMetaData* meta = handle_opt.unwrap_unchecked().meta();
        if (DEV_VERIFY(meta)) {
            DEV_ASSERT(meta->type == AssetType::Scene || meta->type == AssetType::Prefab);
            m_preview_scene = m_scene_reg.registerScene(
                {
                    .source = SceneSource::Editor,
                    .debug_name = meta->name,
                },
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
        auto scene_copy = std::make_unique<Scene>();
        scene_copy->copy(asset->scene());
        return scene_copy;
    }

    return nullptr;
}

}  // namespace cave
