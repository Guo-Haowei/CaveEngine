#include "cave/runtime/ecs/ComponentStorage.h"
#include "cave/runtime/ecs/components/NameComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

#include "engine/private/runtime/ecs/ComponentPool.h"

namespace cave::ecs {

TEST(ComponentStorage, add_get_remove) {
    ecs::ComponentStorage storage;

    storage.GetOrCreate<NameComponent>();

    ecs::Entity e1{ 1 }, e2{ 2 }, e3{ 3 };

    storage.AddDefault(e1, NameComponent_Id);
    storage.AddDefault(e2, NameComponent_Id);
    storage.AddDefault(e3, NameComponent_Id);

    EXPECT_TRUE(storage.Has(e1, NameComponent_Id));
    EXPECT_TRUE(storage.Has(e2, NameComponent_Id));
    EXPECT_TRUE(storage.Has(e3, NameComponent_Id));

    // write distinct values to detect swap bugs
    auto* t1 = (NameComponent*)storage.GetRaw(e1, NameComponent_Id);
    auto* t2 = (NameComponent*)storage.GetRaw(e2, NameComponent_Id);
    auto* t3 = (NameComponent*)storage.GetRaw(e3, NameComponent_Id);

    t1->SetName("111");
    t2->SetName("222");
    t3->SetName("333");

    // remove middle
    EXPECT_TRUE(storage.Remove(e2, NameComponent_Id));

    EXPECT_FALSE(storage.Has(e2, NameComponent_Id));
    EXPECT_TRUE(storage.Has(e1, NameComponent_Id));
    EXPECT_TRUE(storage.Has(e3, NameComponent_Id));

    auto* t1b = (NameComponent*)storage.GetRaw(e1, NameComponent_Id);
    auto* t3b = (NameComponent*)storage.GetRaw(e3, NameComponent_Id);

    EXPECT_TRUE(t1b->GetName() == "111");
    EXPECT_TRUE(t3b->GetName() == "333");
}

}  // namespace cave::ecs
