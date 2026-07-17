#include "SpriteAnimationDocument.h"

#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

// @TODO: remove private #include
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

using namespace ::cave::literals;
using ecs::Entity;

SpriteAnimationDocument::SpriteAnimationDocument(EngineServices& services, const Guid& guid)
    : DocumentBase(services, guid) {

    SceneCommandWriter cb(services.assetRegistry());

    Entity ent = cb.rootObject("animation");
    cb.addComponent(ent, SpriteRendererComponent_Id);
    cb.addComponent(ent, SpriteAnimatorComponent_Id);

    auto scene = MakeOwner<Scene>();

    SceneCommandExecutor executor(*scene);
    EntityMap map(cb.allocationCount());
    SceneCommandPlayback::Play(cb, executor, { map, *scene });
    scene->setRoot(map.resolve(ent));

    SpriteAnimatorComponent* animator = scene->component<SpriteAnimatorComponent>(map.resolve(ent));
    animator->setAnimGuid(guid);

    scene->update(0.0f);

    m_preview_scene = m_scene_reg.registerScene(
        {
            .source = SceneSource::Editor,
            .debug_name = m_handle.meta()->name,
        },
        std::move(scene));
}

}  // namespace cave
