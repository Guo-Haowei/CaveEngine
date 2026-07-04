#include "cave/runtime/ecs/ComponentStorage.h"
#include "cave/runtime/ecs/components/MiscComponents.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

#include "engine/private/runtime/ecs/ComponentPool.h"

namespace cave::ecs {

TEST(ComponentStorage, add_get_remove) {
    ecs::ComponentStorage storage;

    ecs::Entity e1{ 1 }, e2{ 2 }, e3{ 3 };

    storage.createRaw(e1, NameComponent_Id);
    storage.createRaw(e2, NameComponent_Id);
    storage.createRaw(e3, NameComponent_Id);

    EXPECT_TRUE(storage.has(e1, NameComponent_Id));
    EXPECT_TRUE(storage.has(e2, NameComponent_Id));
    EXPECT_TRUE(storage.has(e3, NameComponent_Id));

    // write distinct values to detect swap bugs
    auto* t1 = (NameComponent*)storage.getRaw(e1, NameComponent_Id);
    auto* t2 = (NameComponent*)storage.getRaw(e2, NameComponent_Id);
    auto* t3 = (NameComponent*)storage.getRaw(e3, NameComponent_Id);

    t1->setName("111");
    t2->setName("222");
    t3->setName("333");

    // remove middle
    EXPECT_TRUE(storage.remove(e2, NameComponent_Id));

    EXPECT_FALSE(storage.has(e2, NameComponent_Id));
    EXPECT_TRUE(storage.has(e1, NameComponent_Id));
    EXPECT_TRUE(storage.has(e3, NameComponent_Id));

    auto* t1b = (NameComponent*)storage.getRaw(e1, NameComponent_Id);
    auto* t3b = (NameComponent*)storage.getRaw(e3, NameComponent_Id);

    EXPECT_TRUE(t1b->name() == "111");
    EXPECT_TRUE(t3b->name() == "333");
}

}  // namespace cave::ecs
