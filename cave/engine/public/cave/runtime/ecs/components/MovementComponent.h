// =============================================================================
// File: cave/runtime/platformer/MovementComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Vec.h"
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

    CAVE_PROP(editor = InputFloat)
    float terminal_fall_speed = -30.0f;

    CAVE_PROP(editor = InputFloat)
    float gravity = -35.0f;

    float step_offset = 0.05f;
    float min_ground_support = 0.05f;
};

struct ContactComponent {
    CAVE_COMPONENT(ContactComponent)

    bool hit_left = false;
    bool hit_right = false;
    bool hit_up = false;
    bool hit_down = false;
    math::Vec2f actual_delta = math::Vec2f::Zero;
};

}  // namespace cave
