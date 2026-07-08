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

    const AssetMetaData* meta = nullptr;
    if (auto handle_opt = m_asset_reg.findByGuid<SceneAsset>(guid)) {
        auto handle = handle_opt.unwrap_unchecked();
        meta = handle.meta();
    }
    if (auto handle_opt = m_asset_reg.findByGuid<PrefabAsset>(guid)) {
        auto handle = handle_opt.unwrap_unchecked();
        meta = handle.meta();
    }

    if (DEV_VERIFY(meta)) {
        m_preview_scene = m_scene_reg.registerScene(
            {
                .source = SceneSource::Editor,
                .debug_name = meta->name,
            },
            std::move(scene));
    }
}

SceneDocument::~SceneDocument() {
    if (Scene* scene = m_scene_reg.resolve(m_preview_scene)) {
        scene->end();
    }
}

bool SceneDocument::save() {
    SceneAsset* source = m_handle.get<SceneAsset>();
    Scene* tmp = m_scene_reg.resolve(m_preview_scene);
    source->sceneMut().copy(*tmp);
    return DocumentBase::save();
}

bool SceneDocument::saveAs(std::string_view p_new_path) {
    unused(p_new_path);
    return false;
}

Owner<Scene> SceneDocument::createPreviewScene() const {
    const Scene* scene = nullptr;
    if (const auto scene_asset = m_handle.get<SceneAsset>()) {
        scene = &scene_asset->scene();
    } else if (const auto prefab_asset = m_handle.get<PrefabAsset>()) {
        scene = &prefab_asset->scene();
    }
    if (DEV_VERIFY(scene)) {
        auto scene_copy = std::make_unique<Scene>();
        scene_copy->copy(*scene);
        return scene_copy;
    }
    return nullptr;
}

}  // namespace cave
