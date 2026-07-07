#include "cave/core/ids/GenIdRegistry.h"

#include <memory>

namespace cave {

class TestRegistry : public GenIdRegistry<int> {
public:
    template<typename... Args>
    IdT Create(Args&&... args) {
        return GenIdRegistry<int>::create(
            std::make_unique<int>(std::forward<Args>(args)...));
    }
};

TEST(GenIdRegistry, CreateReturnsAliveAndResolvable) {
    TestRegistry reg;

    auto a = reg.Create(10);

    ASSERT_TRUE(reg.isAlive(a));
    ASSERT_NE(reg.resolve(a), nullptr);
    ASSERT_EQ(*reg.resolve(a), 10);
}

TEST(GenIdRegistry, DestroyMakesIdInvalid) {
    TestRegistry reg;

    auto a = reg.Create(10);

    reg.destroy(a);

    ASSERT_FALSE(reg.isAlive(a));
    ASSERT_EQ(reg.resolve(a), nullptr);
}

TEST(GenIdRegistry, DestroyIsIdempotent) {
    TestRegistry reg;

    auto a = reg.Create(10);

    reg.destroy(a);
    reg.destroy(a);

    ASSERT_FALSE(reg.isAlive(a));
    ASSERT_EQ(reg.resolve(a), nullptr);
}

TEST(GenIdRegistry, ReuseSlotBumpsGeneration) {
    TestRegistry reg;

    auto a = reg.Create(8);
    const uint32_t old_index = a.index;
    const uint32_t old_gen = a.gen;

    reg.destroy(a);

    auto b = reg.Create(9);

    ASSERT_EQ(b.index, old_index);
    ASSERT_NE(b.gen, old_gen);

    ASSERT_FALSE(reg.isAlive(a));
    ASSERT_EQ(reg.resolve(a), nullptr);

    ASSERT_TRUE(reg.isAlive(b));
    ASSERT_NE(reg.resolve(b), nullptr);
    ASSERT_EQ(*reg.resolve(b), 9);
}

TEST(GenIdRegistry, FreeListIsLifo) {
    TestRegistry reg;

    auto a = reg.Create(7);
    auto b = reg.Create(8);
    auto c = reg.Create(9);

    reg.destroy(b);
    reg.destroy(c);

    auto x = reg.Create(10);
    auto y = reg.Create(11);

    ASSERT_EQ(x.index, c.index);
    ASSERT_EQ(y.index, b.index);

    ASSERT_TRUE(reg.isAlive(a));
    ASSERT_TRUE(reg.isAlive(x));
    ASSERT_TRUE(reg.isAlive(y));
}

TEST(GenIdRegistry, DestroyedIdNeverBecomesValidAgain) {
    TestRegistry reg;

    auto a = reg.Create();
    const uint32_t idx = a.index;
    const uint32_t gen = a.gen;

    reg.destroy(a);

    for (int i = 0; i < 10; ++i) {
        auto t = reg.Create();
        reg.destroy(t);
    }

    TestRegistry::IdT stale{ idx, gen };

    ASSERT_FALSE(reg.isAlive(stale));
    ASSERT_EQ(reg.resolve(stale), nullptr);
}

TEST(GenIdRegistry, ReplaceUpdatesStoredObject) {
    TestRegistry reg;

    auto a = reg.Create(1);

    ASSERT_TRUE(reg.replace(a, std::make_unique<int>(42)));

    ASSERT_TRUE(reg.isAlive(a));
    ASSERT_NE(reg.resolve(a), nullptr);
    ASSERT_EQ(*reg.resolve(a), 42);
}

TEST(GenIdRegistry, ReplaceRejectsDeadId) {
    TestRegistry reg;

    auto a = reg.Create(1);
    reg.destroy(a);

    ASSERT_FALSE(reg.replace(a, std::make_unique<int>(42)));
    ASSERT_FALSE(reg.isAlive(a));
}

TEST(GenIdRegistry, ReplaceRejectsNullPointer) {
    TestRegistry reg;

    auto a = reg.Create(1);

    ASSERT_FALSE(reg.replace(a, nullptr));
    ASSERT_TRUE(reg.isAlive(a));
    ASSERT_EQ(*reg.resolve(a), 1);
}

// -----------------------------------------------------------------------------
// Custom deleter tests
// -----------------------------------------------------------------------------

struct DeleterCounter {
    int* delete_count = nullptr;

    void operator()(int* p) const {
        if (p) {
            ++(*delete_count);
            delete p;
        }
    }
};

using CustomPtr = std::unique_ptr<int, DeleterCounter>;

class CustomDeleterRegistry : public GenIdRegistry<int, CustomPtr> {
public:
    explicit CustomDeleterRegistry(int& delete_count)
        : m_delete_count(&delete_count) {}

    IdT Create(int value) {
        return GenIdRegistry<int, CustomPtr>::create(
            CustomPtr{ new int(value), DeleterCounter{ m_delete_count } });
    }

    bool Replace(IdT id, int value) {
        return GenIdRegistry<int, CustomPtr>::replace(
            id,
            CustomPtr{ new int(value), DeleterCounter{ m_delete_count } });
    }

private:
    int* m_delete_count = nullptr;
};

TEST(GenIdRegistry, CustomDeleterIsCalledOnDestroy) {
    int delete_count = 0;
    CustomDeleterRegistry reg{ delete_count };

    auto a = reg.Create(10);

    ASSERT_EQ(delete_count, 0);

    reg.destroy(a);

    ASSERT_EQ(delete_count, 1);
    ASSERT_FALSE(reg.isAlive(a));
}

TEST(GenIdRegistry, CustomDeleterIsCalledOnReplace) {
    int delete_count = 0;
    CustomDeleterRegistry reg{ delete_count };

    auto a = reg.Create(10);

    ASSERT_TRUE(reg.Replace(a, 20));

    ASSERT_EQ(delete_count, 1);
    ASSERT_TRUE(reg.isAlive(a));
    ASSERT_NE(reg.resolve(a), nullptr);
    ASSERT_EQ(*reg.resolve(a), 20);
}

TEST(GenIdRegistry, CustomDeleterIsNotCalledForFailedReplace) {
    int delete_count = 0;
    CustomDeleterRegistry reg{ delete_count };

    auto a = reg.Create(10);
    reg.destroy(a);

    ASSERT_EQ(delete_count, 1);

    ASSERT_FALSE(reg.Replace(a, 20));

    // The temporary replacement pointer should still be destroyed after failed replace.
    ASSERT_EQ(delete_count, 2);
    ASSERT_FALSE(reg.isAlive(a));
}

TEST(GenIdRegistry, CustomDeleterIsCalledForAllLiveSlotsOnDestruction) {
    int delete_count = 0;

    {
        CustomDeleterRegistry reg{ delete_count };

        reg.Create(1);
        reg.Create(2);
        reg.Create(3);

        ASSERT_EQ(delete_count, 0);
    }

    ASSERT_EQ(delete_count, 3);
}

}  // namespace cave
