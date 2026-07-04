#include "cave/runtime/ecs/components/MiscComponents.h"
#include "engine/private/runtime/ecs/ComponentPool.h"

namespace cave::ecs {

TEST(ComponentPool, remove_by_entity) {
    ComponentPool<NameComponent> pool;

    // Create 3 entities with components
    pool.create(Entity(1)) = "a";
    pool.create(Entity(2)) = "b";
    pool.create(Entity(3)) = "c";

    EXPECT_EQ(pool.count(), 3u);
    EXPECT_TRUE(pool.has(Entity(1)));
    EXPECT_TRUE(pool.has(Entity(2)));
    EXPECT_TRUE(pool.has(Entity(3)));

    // Remove middle entity (2)
    pool.remove(Entity(2));

    // Validate state
    EXPECT_FALSE(pool.has(Entity(2)));
    EXPECT_TRUE(pool.has(Entity(1)));
    EXPECT_TRUE(pool.has(Entity(3)));
    EXPECT_EQ(pool.count(), 2u);

    // Ensure no dangling indices and entity IDs match
    for (Entity e : pool.entityArray()) {
        EXPECT_TRUE(pool.has(e));
    }

    // Remove first entity
    pool.remove(Entity(1));
    EXPECT_FALSE(pool.has(Entity(1)));
    EXPECT_TRUE(pool.has(Entity(3)));
    EXPECT_EQ(pool.count(), 1u);

    // Remove last remaining entity
    pool.remove(Entity(3));
    EXPECT_EQ(pool.count(), 0u);
    EXPECT_FALSE(pool.has(Entity(3)));

    // Removing a non-existent entity should be a no-op
    pool.remove(Entity(42));
    EXPECT_EQ(pool.count(), 0u);
}

}  // namespace cave::ecs
