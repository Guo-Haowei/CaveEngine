// =============================================================================
// File: cave/runtime/platformer/VelocityComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Vector.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

struct VelocityComponent {
    CAVE_COMPONENT(VelocityComponent)

    CAVE_PROP(editor = Translation)
    math::Vec3f linear = math::Vec3f::Zero;
};

}  // namespace cave
