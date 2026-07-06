#include "SceneDocument.h"

#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneTickContext.h"

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

    if (auto handle = m_asset_reg.findByGuid<Scene>(guid)) {
        const AssetMetaData* meta = handle.unwrap_unchecked().meta();

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
    Scene* source = m_handle.get<Scene>();
    Scene* tmp = m_scene_reg.resolve(m_preview_scene);
    source->copy(*tmp);
    return DocumentBase::save();
}

bool SceneDocument::saveAs(std::string_view p_new_path) {
    unused(p_new_path);
    return false;
}

std::unique_ptr<Scene> SceneDocument::createPreviewScene() const {
    auto scene = std::make_unique<Scene>();
    scene->copy(*m_handle.get<Scene>());
    return scene;
}

}  // namespace cave
