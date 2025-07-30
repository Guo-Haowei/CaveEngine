#include "engine/ecs/component_manager.inl"

namespace cave {

struct Position {
    float x = 0, y = 0;
};

template<>
struct IsComponent<Position> : std::true_type {};

}  // namespace cave

namespace cave::ecs {

TEST(ComponentManagerTest, RemoveByEntity) {
    ComponentManager<Position> mgr;

    // Create 3 entities with components
    mgr.Create(Entity(1)) = { 1.0f, 1.0f };
    mgr.Create(Entity(2)) = { 2.0f, 2.0f };
    mgr.Create(Entity(3)) = { 3.0f, 3.0f };

    ASSERT_EQ(mgr.GetCount(), 3u);
    EXPECT_TRUE(mgr.Contains(Entity(1)));
    EXPECT_TRUE(mgr.Contains(Entity(2)));
    EXPECT_TRUE(mgr.Contains(Entity(3)));

    // Remove middle entity (2)
    mgr.Remove(Entity(2));

    // Validate state
    EXPECT_FALSE(mgr.Contains(Entity(2)));
    EXPECT_TRUE(mgr.Contains(Entity(1)));
    EXPECT_TRUE(mgr.Contains(Entity(3)));
    EXPECT_EQ(mgr.GetCount(), 2u);

    // Ensure no dangling indices and entity IDs match
    for (Entity e : mgr.GetEntityArray()) {
        EXPECT_TRUE(mgr.Contains(e));
    }

    // Remove first entity
    mgr.Remove(Entity(1));
    EXPECT_FALSE(mgr.Contains(Entity(1)));
    EXPECT_TRUE(mgr.Contains(Entity(3)));
    EXPECT_EQ(mgr.GetCount(), 1u);

    // Remove last remaining entity
    mgr.Remove(Entity(3));
    EXPECT_EQ(mgr.GetCount(), 0u);
    EXPECT_FALSE(mgr.Contains(Entity(3)));

    // Removing a non-existent entity should be a no-op
    mgr.Remove(Entity(42));
    EXPECT_EQ(mgr.GetCount(), 0u);
}

}  // namespace cave::ecs
