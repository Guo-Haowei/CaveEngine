// =============================================================================
// File: cave/runtime/ui/UITypes.h
// =============================================================================
#pragma once
#include "cave/core/Color.h"
#include "cave/core/math/Box.h"
#include "cave/core/math/Rect.h"

namespace cave {

using OldUIRect = math::Rect<float>;

using UIId = uint64_t;
using UIColor = Color;
using UIRect = math::Box2;

}  // namespace cave
