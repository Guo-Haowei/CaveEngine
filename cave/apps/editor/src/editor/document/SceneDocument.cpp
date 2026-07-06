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
        .native_scripts = services.nativeScripts(),
        .scene = *scene,
        .scene_transition = *(ISceneTransitionRequests*)(nullptr),
        .query = SceneQuery(*scene),
        .view_id = {},
        .engine_services = services,
    };
    scene->begin({
        .domain = SceneTickDomain::Editor,
        .dt = 0.0f,
        .scene_ctx = ctx,
    });

    preview_scene_ = scene_reg_.registerScene(std::move(scene));
}

bool SceneDocument::save() {
    Scene* source = handle_.get<Scene>();
    Scene* tmp = scene_reg_.resolve(preview_scene_);
    source->copy(*tmp);
    return DocumentBase::save();
}

bool SceneDocument::saveAs(std::string_view p_new_path) {
    unused(p_new_path);
    return false;
}

std::unique_ptr<Scene> SceneDocument::createPreviewScene() const {
    auto scene = std::make_unique<Scene>(std::format("preview-scene-{}", guid().toString()));
    scene->copy(*handle_.get<Scene>());
    return scene;
}

}  // namespace cave
