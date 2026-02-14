#include "cave/runtime/ecs/ComponentStorage.h"

#include "engine/private/runtime/ecs/ComponentPool.h"

struct Dummy {
    int debug;
};

template<>
struct ::cave::IsComponent<Dummy> : std::true_type {};

namespace cave::ecs {

constexpr ComponentId Dummy_Id = 1;

TEST(ComponentStorage, add_get_remove) {
    ecs::ComponentStorage storage;

    storage.RegisterTyped<Dummy>(Dummy_Id);

    ecs::Entity e1{ 1 }, e2{ 2 }, e3{ 3 };

    storage.AddDefault(e1, Dummy_Id);
    storage.AddDefault(e2, Dummy_Id);
    storage.AddDefault(e3, Dummy_Id);

    EXPECT_TRUE(storage.Has(e1, Dummy_Id));
    EXPECT_TRUE(storage.Has(e2, Dummy_Id));
    EXPECT_TRUE(storage.Has(e3, Dummy_Id));

    // write distinct values to detect swap bugs
    auto* t1 = (Dummy*)storage.GetRaw(e1, Dummy_Id);
    auto* t2 = (Dummy*)storage.GetRaw(e2, Dummy_Id);
    auto* t3 = (Dummy*)storage.GetRaw(e3, Dummy_Id);

    t1->debug = 111;
    t2->debug = 222;
    t3->debug = 333;

    // remove middle
    EXPECT_TRUE(storage.Remove(e2, Dummy_Id));

    EXPECT_FALSE(storage.Has(e2, Dummy_Id));
    EXPECT_TRUE(storage.Has(e1, Dummy_Id));
    EXPECT_TRUE(storage.Has(e3, Dummy_Id));

    auto* t1b = (Dummy*)storage.GetRaw(e1, Dummy_Id);
    auto* t3b = (Dummy*)storage.GetRaw(e3, Dummy_Id);

    EXPECT_TRUE(t1b->debug == 111);
    EXPECT_TRUE(t3b->debug == 333);
}

}  // namespace cave::ecs
