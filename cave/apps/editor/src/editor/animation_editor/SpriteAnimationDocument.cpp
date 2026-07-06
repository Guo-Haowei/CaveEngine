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
    Entity root = cb.rootObject();

    Entity ent = cb.transformObject("animation");
    cb.attachChild(ent, root);
    cb.addComponent(ent, SpriteRendererComponent_Id);
    cb.addComponent(ent, SpriteAnimatorComponent_Id);

    auto scene = std::make_unique<Scene>(std::format("preview-animation-{}", guid.toString()));

    SceneCommandExecutor executor(*scene);
    EntityMap map(cb.allocationCount());
    SceneCommandPlayback::Play(cb, executor, { map, *scene });
    scene->setRoot(map.Resolve(root));

#pragma warning(push)
#pragma warning(disable : 4996)
    scene->update(0.0f);
#pragma warning(pop)

    SpriteAnimatorComponent* animator = scene->component<SpriteAnimatorComponent>(map.Resolve(ent));
    animator->SetResourceGuid(guid);

    preview_scene_ = scene_reg_.registerScene(std::move(scene));
}

}  // namespace cave
