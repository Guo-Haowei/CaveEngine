#pragma once
#include <cstdint>
#include "cave/runtime/ecs/Entity.h"

namespace cave {

struct WorldChange {
    enum class Kind : uint8_t {
        EntityCreated,
        EntityDestroyed,
        ComponentAdded,
        ComponentRemoved,
        PropertySet,
        ArrayInsert,
        ArrayErase,
        ParentChanged,
    } kind;

    ecs::Entity entity{};
    // ComponentId component{};
    // PropertyId property{};
};

}  // namespace cave
