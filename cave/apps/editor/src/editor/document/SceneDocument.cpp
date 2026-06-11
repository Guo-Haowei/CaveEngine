#include "SceneDocument.h"

#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

SceneDocument::SceneDocument(IApplication& p_app, const Guid& p_guid)
    : DocumentBase(p_app, p_guid) {

    auto scene = std::make_unique<Scene>(std::format("preview-scene-{}", p_guid.ToString()));
    scene->Copy(*handle_.Get<Scene>());

    preview_scene_ = scene_reg_.registerScene(std::move(scene));
}

bool SceneDocument::save() {
    Scene* source = handle_.Get<Scene>();
    Scene* tmp = scene_reg_.resolve(preview_scene_);
    source->Copy(*tmp);
    return asset_reg_.SaveAsset(guid_);
}

bool SceneDocument::saveAs(std::string_view p_new_path) {
    unused(p_new_path);
    return false;
}

}  // namespace cave
