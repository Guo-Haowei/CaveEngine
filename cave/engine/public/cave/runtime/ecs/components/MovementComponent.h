// =============================================================================
// File: cave/runtime/platformer/MovementComponent.h
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

struct MotorComponent {
    CAVE_COMPONENT(MotorComponent)

    CAVE_PROP(editor = Toggle)
    bool affected_by_gravity = false;

    CAVE_PROP(editor = Translation)
    math::Vec3f gravity = math::Vec3f{ 0.0f, -35.0f, 0.0f };
};

}  // namespace cave
