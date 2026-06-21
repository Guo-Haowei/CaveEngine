// =============================================================================
// File: cave/runtime/platformer/PlatformerMotorComponent.h
// =============================================================================
#pragma once

namespace cave {

struct PlatformerMotorComponent {
    CAVE_COMPONENT(PlatformerMotorComponent)

    CAVE_PROP()
    float move_speed = 5.5f;

    CAVE_PROP()
    float gravity = -35.0f;

    // Non-Serialized
    bool grounded = false;
    bool landed_this_frame = false;

    bool hit_left = false;
    bool hit_right = false;
    bool hit_ceiling = false;
};

}  // namespace cave
