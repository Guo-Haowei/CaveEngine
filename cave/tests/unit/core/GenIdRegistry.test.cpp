#include "engine/private/core/ids/GenIdRegistry.h"

namespace cave {

class TestRegistry : public GenIdRegistry<int> {
public:
    template<typename... Args>
    IdT Create(Args&&... args) {
        return GenIdRegistry<int>::create(std::make_unique<int>(std::forward<Args>(args)...));
    }
};

TEST(GenIdRegistry, create_returns_alive_and_resolvable) {
    TestRegistry reg;

    auto a = reg.Create(10);
    ASSERT_TRUE(reg.isAlive(a));
    ASSERT_NE(reg.resolve(a), nullptr);

    reg.destroy(a);
    ASSERT_FALSE(reg.isAlive(a));
    ASSERT_EQ(reg.resolve(a), nullptr);
}

TEST(GenIdRegistry, destroy_is_idempotent) {
    TestRegistry reg;

    auto a = reg.Create();
    ASSERT_TRUE(reg.isAlive(a));

    reg.destroy(a);
    ASSERT_FALSE(reg.isAlive(a));

    // Destroy again should not crash, should remain dead.
    reg.destroy(a);
    ASSERT_FALSE(reg.isAlive(a));
    ASSERT_EQ(reg.resolve(a), nullptr);
}

TEST(GenIdRegistry, reuse_slot_bumps_generation) {
    TestRegistry reg;

    auto a = reg.Create(8);
    ASSERT_TRUE(reg.isAlive(a));

    const uint32_t oldIndex = a.index;
    const uint32_t oldGen = a.gen;

    reg.destroy(a);
    ASSERT_FALSE(reg.isAlive(a));

    // Next allocation should reuse the freed slot (your allocator is free-list based).
    auto b = reg.Create(9);

    ASSERT_EQ(b.index, oldIndex) << "Expected slot reuse";
    ASSERT_NE(b.gen, oldGen) << "Generation must change on reuse to invalidate stale IDs";

    // Old ID must remain invalid.
    ASSERT_FALSE(reg.isAlive(a));
    ASSERT_EQ(reg.resolve(a), nullptr);

    // New ID must be valid.
    ASSERT_TRUE(reg.isAlive(b));
    ASSERT_NE(reg.resolve(b), nullptr);
}

TEST(GenIdRegistry, free_list_is_lifo) {
    TestRegistry reg;

    auto a = reg.Create(7);
    auto b = reg.Create(8);
    auto c = reg.Create(9);

    reg.destroy(b);
    reg.destroy(c);

    // If free-list is LIFO (push_back / pop_back), next allocation should reuse 'c' first.
    auto x = reg.Create(10);
    ASSERT_EQ(x.index, c.index);

    // Next should reuse 'b'.
    auto y = reg.Create(11);
    ASSERT_EQ(y.index, b.index);

    // Clean up
    reg.destroy(a);
    reg.destroy(x);
    reg.destroy(y);
}

TEST(GenIdRegistry, destroyed_id_never_becomes_valid_again) {
    TestRegistry reg;

    auto a = reg.Create();
    const uint32_t idx = a.index;
    const uint32_t gen = a.gen;

    reg.destroy(a);

    // Allocate/destroy a bunch to force multiple reuses of the same slot.
    for (int i = 0; i < 10; ++i) {
        auto t = reg.Create();
        reg.destroy(t);
    }

    // Even if the index is reused, the original generation must not match again.
    // (If gen overflows eventually, this could fail after billions of frees; acceptable.)
    TestRegistry::IdT stale{ idx, gen };
    ASSERT_FALSE(reg.isAlive(stale));
    ASSERT_EQ(reg.resolve(stale), nullptr);
}

}  // namespace cave
