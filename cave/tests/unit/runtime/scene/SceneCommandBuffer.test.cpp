#include "cave/runtime/scene/SceneCommandBuffer.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using ecs::Entity;
using math::Vector3f;

#if 0
TEST(SceneCommandBuffer, add_component) {
    Scene scene;

    SceneCommandBuffer ecb;
    Entity temp = ecb.Create();
    std::string_view name = "MyTestComponent";

    ecb.Add(temp, NameComponent_Id);
    ecb.SetName(temp, name);
    ecb.Playback(scene);

    Entity real = ecb.Resolve(temp);
    EXPECT_TRUE(scene.Contains<NameComponent>(real));
    const NameComponent& name_component = *scene.GetComponent<NameComponent>(real);
    EXPECT_EQ(name_component.GetName(), name);
}

TEST(SceneCommandBuffer, add_component_to_existing_entity) {
    Scene scene;
    Entity e = scene.CreateEntity();
    scene.Create<NameComponent>(e).SetName("entity from scene");

    SceneCommandBuffer ecb;

    Vector3f scale = Vector3f::UnitX;
    ecb.Add(e, TransformComponent_Id);
    ecb.SetScale(e, scale);

    ecb.Playback(scene);
    Entity real = ecb.Resolve(e);
    EXPECT_EQ(real, e);
    EXPECT_TRUE(scene.Contains<TransformComponent>(real));
    const TransformComponent& transform = *scene.GetComponent<TransformComponent>(real);
    EXPECT_EQ(transform.GetScale(), scale);
}
#endif

}  // namespace cave
