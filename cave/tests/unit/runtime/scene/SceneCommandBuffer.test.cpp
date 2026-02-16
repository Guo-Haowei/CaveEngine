#include "cave/runtime/ecs/ComponentRegistry.h"
#include "cave/runtime/ecs/components/HierarchyComponent.h"
#include "cave/runtime/scene/SceneCommandBuffer.h"
#include "cave/runtime/scene/SceneCommandExecutor.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave::scene {

using namespace ecs;

TEST(SceneCommandBuffer, playback_should_resolve_temp_entity) {
    SceneCommandBuffer cb;

    Scene scene("test");

    Entity e1 = cb.CreateEntity();
    Entity e2 = cb.CreateEntity();

    cb.AddComponent(e1, NameComponent_Id);
    cb.SetProperty(e1, NameComponent_Id, StringId("name"), FixedString<64>("e1"));

    cb.AddComponent(e2, NameComponent_Id);
    cb.AddComponent(e2, HierarchyComponent_Id);

    cb.SetProperty(e2, NameComponent_Id, StringId("name"), FixedString<64>("e2"));
    cb.SetProperty(e2, HierarchyComponent_Id, StringId("parent_id"), e1);

    ComponentRegistry reg = ComponentRegistry::Builtin();
    SceneCommandExecutor mut(scene, reg);
    cb.Playback(mut);

    Entity r1 = cb.Resolve(e1);
    Entity r2 = cb.Resolve(e2);

    const HierarchyComponent* hier2 = scene.GetComponent<HierarchyComponent>(r2);
    ASSERT_TRUE(hier2);

    EXPECT_EQ(hier2->parent_id, r1);
    EXPECT_NE(hier2->parent_id, e1);
}

}  // namespace cave::scene
