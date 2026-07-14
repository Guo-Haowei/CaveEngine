// =============================================================================
// File: cave/runtime/ui/UIInput.h
// =============================================================================
#pragma once
#include "cave/core/math/Vec.h"

namespace cave {

struct UIInput {
    math::Vec2f cursor_os{ 0.0f, 0.0f };  // cursor in screen space

    bool submit_pressed = false;
    bool submit_released = false;
    bool submit_down = false;

    bool up_pressed = false;
    bool down_pressed = false;
    bool left_pressed = false;
    bool right_pressed = false;
};

}  // namespace cave
