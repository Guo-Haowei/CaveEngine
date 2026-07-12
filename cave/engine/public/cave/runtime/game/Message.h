// =============================================================================
// File: cave/runtime/game/Message.h
// =============================================================================
#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/containers/Containers.h"
#include "cave/core/string/StringId.h"
#include "cave/core/variant/Variant.h"

namespace cave {

struct Message {
    StringId id;
    ecs::Entity sender;
    Variant payload;
};

}  // namespace cave
