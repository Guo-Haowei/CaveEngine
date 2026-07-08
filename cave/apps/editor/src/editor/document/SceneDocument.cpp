#include "SceneDocument.h"

#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneTickContext.h"

#include "engine/private/runtime/assets/SceneAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/scene/Scene.h"

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

    if (auto handle_opt = m_asset_reg.findByGuid<SceneAsset>(guid)) {
        auto handle = handle_opt.unwrap_unchecked();
        const AssetMetaData* meta = handle.meta();
        DEV_ASSERT(meta);
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
    const SceneAsset* asset = m_handle.get<SceneAsset>();
    if (!asset) {
        return nullptr;
    }

    auto scene = std::make_unique<Scene>();
    scene->copy(asset->scene());
    return scene;
}

}  // namespace cave
