#include "SpriteAnimationDocument.h"

#include "cave/runtime/framework/EngineServices.h"
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
    Entity root = cb.CreateRootObject();

    Entity ent = cb.CreateTransformObject("animation");
    cb.AttachChild(ent, root);
    cb.AddComponent(ent, SpriteRendererComponent_Id);
    cb.AddComponent(ent, SpriteAnimatorComponent_Id);
    cb.SetProperty(ent, SpriteAnimatorComponent_Id, "anim_id"_sid, guid);

    auto scene = std::make_unique<Scene>(std::format("preview-animation-{}", guid.ToString()));

    SceneCommandExecutor executor(*scene);
    EntityMap map(cb.GetAllocationCount());
    SceneCommandPlayback::Play(cb, executor, { map, *scene });
    scene->m_root = map.Resolve(root);
    scene->update(0.0f);

    preview_scene_ = scene_reg_.registerScene(std::move(scene));
}

}  // namespace cave
