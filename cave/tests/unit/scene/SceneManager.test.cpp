#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneManager.h"

namespace cave {

static SceneDesc MakeDesc(const char* p_name) {
    unused(p_name);
    return {
#if USING(DEBUG_BUILD)
        p_name
#endif
    };
}

TEST(SceneManager, create_returns_alive_and_resolvable) {
    SceneManager sm;

    SceneId a = sm.Create(MakeDesc("A"));
    ASSERT_TRUE(sm.IsAlive(a));
    ASSERT_NE(sm.Resolve(a), nullptr);

    sm.Destroy(a);
    ASSERT_FALSE(sm.IsAlive(a));
    ASSERT_EQ(sm.Resolve(a), nullptr);
}

TEST(SceneManager, destroy_is_idempotent) {
    SceneManager sm;

    SceneId a = sm.Create(MakeDesc("A"));
    ASSERT_TRUE(sm.IsAlive(a));

    sm.Destroy(a);
    ASSERT_FALSE(sm.IsAlive(a));

    // Destroy again should not crash, should remain dead.
    sm.Destroy(a);
    ASSERT_FALSE(sm.IsAlive(a));
    ASSERT_EQ(sm.Resolve(a), nullptr);
}

TEST(SceneManager, reuse_slot_bumps_generation) {
    SceneManager sm;

    SceneId a = sm.Create(MakeDesc("A"));
    ASSERT_TRUE(sm.IsAlive(a));

    const uint32_t oldIndex = a.index;
    const uint32_t oldGen = a.gen;

    sm.Destroy(a);
    ASSERT_FALSE(sm.IsAlive(a));

    // Next allocation should reuse the freed slot (your allocator is free-list based).
    SceneId b = sm.Create(MakeDesc("B"));

    ASSERT_EQ(b.index, oldIndex) << "Expected slot reuse";
    ASSERT_NE(b.gen, oldGen) << "Generation must change on reuse to invalidate stale IDs";

    // Old ID must remain invalid.
    ASSERT_FALSE(sm.IsAlive(a));
    ASSERT_EQ(sm.Resolve(a), nullptr);

    // New ID must be valid.
    ASSERT_TRUE(sm.IsAlive(b));
    ASSERT_NE(sm.Resolve(b), nullptr);
}

TEST(SceneManager, register_also_uses_allocation_and_produces_alive_scene) {
    SceneManager sm;

    auto scene = std::make_unique<Scene>();  // or DummyScene / Scene(desc) depending on your ctor
    SceneId id = sm.Register(std::move(scene), MakeDesc("Registered"));

    ASSERT_TRUE(sm.IsAlive(id));
    ASSERT_NE(sm.Resolve(id), nullptr);

    sm.Destroy(id);
    ASSERT_FALSE(sm.IsAlive(id));
    ASSERT_EQ(sm.Resolve(id), nullptr);
}

TEST(SceneManager, free_list_is_lifo) {
    SceneManager sm;

    SceneId a = sm.Create(MakeDesc("A"));
    SceneId b = sm.Create(MakeDesc("B"));
    SceneId c = sm.Create(MakeDesc("C"));

    sm.Destroy(b);
    sm.Destroy(c);

    // If free-list is LIFO (push_back / pop_back), next allocation should reuse 'c' first.
    SceneId x = sm.Create(MakeDesc("X"));
    ASSERT_EQ(x.index, c.index);

    // Next should reuse 'b'.
    SceneId y = sm.Create(MakeDesc("Y"));
    ASSERT_EQ(y.index, b.index);

    // Clean up
    sm.Destroy(a);
    sm.Destroy(x);
    sm.Destroy(y);
}

TEST(SceneManager, destroyed_id_never_becomes_valid_again) {
    SceneManager sm;

    SceneId a = sm.Create(MakeDesc("A"));
    const uint32_t idx = a.index;
    const uint32_t gen = a.gen;

    sm.Destroy(a);

    // Allocate/destroy a bunch to force multiple reuses of the same slot.
    for (int i = 0; i < 10; ++i) {
        SceneId t = sm.Create(MakeDesc("Tmp"));
        sm.Destroy(t);
    }

    // Even if the index is reused, the original generation must not match again.
    // (If gen overflows eventually, this could fail after billions of frees; acceptable.)
    SceneId stale{ idx, gen };
    ASSERT_FALSE(sm.IsAlive(stale));
    ASSERT_EQ(sm.Resolve(stale), nullptr);
}

}  // namespace cave
