// =============================================================================
// File: cave/runtime/input/PointerState.h
// =============================================================================
#pragma once
#include "cave/core/math/Vec.h"

namespace cave {

struct PointerState {
    bool has_pos = false;
    math::Vec2f pos_win;  // position in window space
    math::Vec2f delta;
};

}  // namespace cave
