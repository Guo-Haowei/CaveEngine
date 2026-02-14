#include "engine/private/runtime/ecs/ComponentPool.h"

struct Position {
    float x = 0, y = 0;
};

template<>
struct ::cave::IsComponent<Position> : std::true_type {};

namespace cave::ecs {

TEST(ComponentPool, remove_by_entity) {
    ComponentPool<Position> pool;

    // Create 3 entities with components
    pool.Create(Entity(1)) = { 1.0f, 1.0f };
    pool.Create(Entity(2)) = { 2.0f, 2.0f };
    pool.Create(Entity(3)) = { 3.0f, 3.0f };

    ASSERT_EQ(pool.GetCount(), 3u);
    EXPECT_TRUE(pool.Contains(Entity(1)));
    EXPECT_TRUE(pool.Contains(Entity(2)));
    EXPECT_TRUE(pool.Contains(Entity(3)));

    // Remove middle entity (2)
    pool.Remove(Entity(2));

    // Validate state
    EXPECT_FALSE(pool.Contains(Entity(2)));
    EXPECT_TRUE(pool.Contains(Entity(1)));
    EXPECT_TRUE(pool.Contains(Entity(3)));
    EXPECT_EQ(pool.GetCount(), 2u);

    // Ensure no dangling indices and entity IDs match
    for (Entity e : pool.GetEntityArray()) {
        EXPECT_TRUE(pool.Contains(e));
    }

    // Remove first entity
    pool.Remove(Entity(1));
    EXPECT_FALSE(pool.Contains(Entity(1)));
    EXPECT_TRUE(pool.Contains(Entity(3)));
    EXPECT_EQ(pool.GetCount(), 1u);

    // Remove last remaining entity
    pool.Remove(Entity(3));
    EXPECT_EQ(pool.GetCount(), 0u);
    EXPECT_FALSE(pool.Contains(Entity(3)));

    // Removing a non-existent entity should be a no-op
    pool.Remove(Entity(42));
    EXPECT_EQ(pool.GetCount(), 0u);
}

}  // namespace cave::ecs
