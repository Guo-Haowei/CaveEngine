// =============================================================================
// File: cave/runtime/input/PointerState.h
// =============================================================================
#pragma once
#include "cave/core/math/Vector.h"

namespace cave {

struct PointerState {
    bool has_pos = false;
    math::Vector2f pos_win;  // position in window space
    math::Vector2f delta;
};

}  // namespace cave
