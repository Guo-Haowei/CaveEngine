#include "MaterialDocument.h"

#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/ecs/components/MaterialComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/scene/SceneMutator.h"
#include "cave/runtime/scene/SceneMutatorExt.h"

#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

using ecs::Entity;

MaterialDocument::MaterialDocument(IApplication& p_app, const Guid& p_guid)
    : DocumentBase(p_app, p_guid) {

    SceneCommandBuffer cb;
    SceneExt scene_ext(*p_app.GetAssetRegistry());
    Entity root = scene_ext.CreateRootObject(cb);

    if constexpr (1) {
        Entity light = scene_ext.CreatePointLightObject(cb, "point_light", math::Vector3f(0, 3, 1));
        scene_ext.AttachChild(cb, light, root);
    }

    if constexpr (1) {
        Entity sphere = scene_ext.CreateSphereObject(cb, "sphere", &p_guid);
        scene_ext.AttachChild(cb, sphere, root);
    }

    auto scene = std::make_unique<Scene>(std::format("preview-material-{}", p_guid.ToString()));
    SceneMutator mut(*scene);
    cb.Playback(mut);

    scene->m_root = cb.Resolve(root);
    scene->Update(0.0f);

    m_preview_scene = m_scene_reg.Register(std::move(scene));
}

}  // namespace cave
