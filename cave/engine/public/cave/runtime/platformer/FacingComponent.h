// =============================================================================
// File: cave/runtime/platformer/FacingComponent.h
// =============================================================================
#pragma once
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

enum class Facing : uint8_t {
    Left = 0,
    Right,
    Top,
    Bottom,

    Count,
};

DECLARE_ENUM_TRAITS(Facing, "left", "right", "top", "bottom");

struct FacingComponent {
    CAVE_COMPONENT(FacingComponent)

    CAVE_PROP(editor = EnumDropDown)
    Facing facing = Facing::Right;

    bool operator==(const FacingComponent& rhs) const {
        return facing == rhs.facing;
    }

    bool operator!=(const FacingComponent& rhs) const {
        return facing != rhs.facing;
    }
};

}  // namespace cave
