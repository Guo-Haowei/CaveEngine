#include "cave/runtime/ecs/ComponentStorage.h"
#include "cave/runtime/ecs/components/MiscComponents.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

#include "engine/private/runtime/ecs/ComponentPool.h"

namespace cave::ecs {

TEST(ComponentStorage, add_get_remove) {
    ecs::ComponentStorage storage;

    ecs::Entity e1{ 1 }, e2{ 2 }, e3{ 3 };

    storage.createRaw(NameComponent_Id, e1);
    storage.createRaw(NameComponent_Id, e2);
    storage.createRaw(NameComponent_Id, e3);

    EXPECT_TRUE(storage.has(NameComponent_Id, e1));
    EXPECT_TRUE(storage.has(NameComponent_Id, e2));
    EXPECT_TRUE(storage.has(NameComponent_Id, e3));

    // write distinct values to detect swap bugs
    auto* t1 = (NameComponent*)storage.getRaw(NameComponent_Id, e1);
    auto* t2 = (NameComponent*)storage.getRaw(NameComponent_Id, e2);
    auto* t3 = (NameComponent*)storage.getRaw(NameComponent_Id, e3);

    t1->setName("111");
    t2->setName("222");
    t3->setName("333");

    // remove middle
    EXPECT_TRUE(storage.remove(NameComponent_Id, e2));

    EXPECT_FALSE(storage.has(NameComponent_Id, e2));
    EXPECT_TRUE(storage.has(NameComponent_Id, e1));
    EXPECT_TRUE(storage.has(NameComponent_Id, e3));

    auto* t1b = (NameComponent*)storage.getRaw(NameComponent_Id, e1);
    auto* t3b = (NameComponent*)storage.getRaw(NameComponent_Id, e3);

    EXPECT_TRUE(t1b->name() == "111");
    EXPECT_TRUE(t3b->name() == "333");
}

}  // namespace cave::ecs
