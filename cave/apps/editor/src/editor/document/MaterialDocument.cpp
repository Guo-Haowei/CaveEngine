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

MaterialDocument::MaterialDocument(IApplication& app, const Guid& guid)
    : DocumentBase(app, guid) {

    SceneCommandWriter cb(*app.GetAssetRegistry());
    Entity root = cb.CreateRootObject();

    if constexpr (1) {
        Entity light = cb.CreatePointLightObject("point_light", math::Vector3f(0, 3, 1));
        cb.AttachChild(light, root);
    }

    if constexpr (1) {
        Entity sphere = cb.CreateSphereObject("sphere", { &guid });
        cb.AttachChild(sphere, root);
    }

    auto scene = std::make_unique<Scene>(std::format("preview-material-{}", guid.ToString()));

    SceneCommandExecutor executor(*scene);
    EntityMap map(cb.GetAllocationCount());
    SceneCommandPlayback::Play(cb, executor, { map, *scene });
    scene->m_root = map.Resolve(root);
    scene->Update(0.0f);

    m_preview_scene = m_scene_reg.registerScene(std::move(scene));
}

}  // namespace cave
