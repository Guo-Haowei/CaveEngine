// =============================================================================
// File: engine/public/cave/ui/UIInput.h
// =============================================================================
#pragma once
#include "cave/core/math/Vector.h"

namespace cave {

struct UIInput {
    math::Vector2f mouse_pos{ 0.0f, 0.0f };

    bool mouse_down = false;
    bool mouse_pressed = false;
    bool mouse_released = false;

    bool submit_pressed = false;
    bool cancel_pressed = false;

    bool up_pressed = false;
    bool down_pressed = false;
    bool left_pressed = false;
    bool right_pressed = false;
};

}  // namespace cave
