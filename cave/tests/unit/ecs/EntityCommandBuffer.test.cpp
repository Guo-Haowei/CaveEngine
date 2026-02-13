#include "cave/runtime/ecs/EntityCommandBuffer.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave::ecs {

using namespace math;

TEST(EntityCommandBuffer, add_component) {
    Scene scene;

    EntityCommandBuffer ecb;
    Entity temp = ecb.Create();
    std::string_view name = "MyTestComponent";

    {
        NameComponent name_component;
        name_component.SetName(name);
        ecb.Add(temp, NameComponent_Id, name_component);
    }

    ecb.Playback(scene);

    Entity real = ecb.Resolve(temp);
    EXPECT_TRUE(scene.Contains<NameComponent>(real));
    const NameComponent& name_component = *scene.GetComponent<NameComponent>(real);
    EXPECT_EQ(name_component.GetName(), name);
}

TEST(EntityCommandBuffer, add_component_to_existing_entity) {
    Scene scene;
    Entity e = scene.CreateEntity();
    scene.Create<NameComponent>(e).SetName("entity from scene");

    EntityCommandBuffer ecb;

    Vector3f scale = Vector3f::UnitX;
    {
        TransformComponent transform;
        transform.SetScale(scale);
        ecb.Add(e, TransformComponent_Id, transform);
    }

    ecb.Playback(scene);

    Entity real = ecb.Resolve(e);
    EXPECT_EQ(real, e);
    EXPECT_TRUE(scene.Contains<TransformComponent>(real));
    const TransformComponent& transform = *scene.GetComponent<TransformComponent>(real);
    EXPECT_EQ(transform.GetScale(), scale);
}

}  // namespace cave::ecs
