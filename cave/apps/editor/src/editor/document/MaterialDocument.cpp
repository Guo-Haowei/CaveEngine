#include "MaterialDocument.h"

#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/ecs/components/MaterialComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

// @TODO: no private include
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

using ecs::Entity;

MaterialDocument::MaterialDocument(EngineServices& services, const Guid& guid)
    : DocumentBase(services, guid) {

    SceneCommandWriter cb(services.assetRegistry());
    Entity root = cb.rootObject();

    if constexpr (1) {
        Entity light = cb.pointLightObject("point_light", math::Vec3f(0, 3, 1));
        cb.attachChild(light, root);
    }

    if constexpr (1) {
        Entity sphere = cb.sphereObject("sphere", { &guid });
        cb.attachChild(sphere, root);
    }

    auto scene = std::make_unique<Scene>(std::format("preview-material-{}", guid.toString()));

    SceneCommandExecutor executor(*scene);
    EntityMap map(cb.allocationCount());
    SceneCommandPlayback::Play(cb, executor, { map, *scene });
    scene->setRoot(map.Resolve(root));
    scene->update(0.0f);

    m_preview_scene = m_scene_reg.registerScene(std::move(scene));
}

}  // namespace cave
