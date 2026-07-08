#include "cave/runtime/ecs/ComponentRegistry.h"
#include "cave/runtime/ecs/components/HierarchyComponent.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"

#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave::scene {

using namespace cave::literals;
using namespace ecs;

TEST(SceneCommandBuffer, playback_should_resolve_temp_entity) {
    SceneCommandBuffer cb;

    Scene scene;

    Entity e1 = cb.createEntity();
    Entity e2 = cb.createEntity();

    cb.addComponent(e1, NameComponent_Id);
    cb.setProperty(e1, NameComponent_Id, "name"_sid, FixedString<64>("e1"));

    cb.addComponent(e2, NameComponent_Id);
    cb.addComponent(e2, HierarchyComponent_Id);

    cb.setProperty(e2, NameComponent_Id, "name"_sid, FixedString<64>("e2"));
    cb.setProperty(e2, HierarchyComponent_Id, "parent_id"_sid, e1);

    ComponentRegistry reg = ComponentRegistry::builtin();

    SceneCommandExecutor executor(scene, reg);
    EntityMap map(cb.allocationCount());
    SceneCommandPlayback::Play(cb, executor, { map, scene });

    Entity r1 = map.resolve(e1);
    Entity r2 = map.resolve(e2);

    const HierarchyComponent* hier2 = scene.component<HierarchyComponent>(r2);
    ASSERT_TRUE(hier2);

    EXPECT_EQ(hier2->parent_id, r1);
    EXPECT_NE(hier2->parent_id, e1);
}

}  // namespace cave::scene
