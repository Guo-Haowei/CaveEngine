#include "SceneDocument.h"

#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

SceneDocument::SceneDocument(IApplication& p_app, const Guid& p_guid)
    : DocumentBase(p_app, p_guid) {

    auto scene = std::make_unique<Scene>(std::format("preview-scene-{}", p_guid.ToString()));
    scene->Copy(*m_handle.Get<Scene>());

    m_preview_scene = m_scene_reg.registerScene(std::move(scene));
}

bool SceneDocument::Save() {
    Scene* source = m_handle.Get<Scene>();
    Scene* tmp = m_scene_reg.resolve(m_preview_scene);
    source->Copy(*tmp);
    return m_asset_reg.SaveAsset(m_guid);
}

bool SceneDocument::SaveAs(std::string_view p_new_path) {
    unused(p_new_path);
    return false;
}

}  // namespace cave
