#include "MaterialDocument.h"

#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/ecs/components/MaterialComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

using ecs::Entity;

MaterialDocument::MaterialDocument(IApplication& p_app, const Guid& p_guid)
    : DocumentBase(p_app, p_guid) {

    SceneCommandWriter cb(*p_app.GetAssetRegistry());
    Entity root = cb.CreateRootObject();

    if constexpr (1) {
        Entity light = cb.CreatePointLightObject("point_light", math::Vector3f(0, 3, 1));
        cb.AttachChild(light, root);
    }

    if constexpr (1) {
        Entity sphere = cb.CreateSphereObject("sphere", &p_guid);
        cb.AttachChild(sphere, root);
    }

    auto scene = std::make_unique<Scene>(std::format("preview-material-{}", p_guid.ToString()));

    SceneCommandExecutor executor(*scene);
    EntityMap map(cb.GetAllocationCount());
    SceneCommandPlayback(cb, executor, map);
    scene->m_root = map.Resolve(root);
    scene->Update(0.0f);

    m_preview_scene = m_scene_reg.Register(std::move(scene));
}

}  // namespace cave
