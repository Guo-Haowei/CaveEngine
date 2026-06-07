// =============================================================================
// File: cave/runtime/view/ViewRecord.h
// =============================================================================
#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/core/math/Rect.h"
#include "cave/core/math/Vector.h"

namespace cave {

struct ViewRecord {
    ViewId view_id;

    std::string debug_name;

    // Where the view is displayed inside the window.
    // Used for input hit testing
    // The x, y are in OS/screen space
    math::FloatRect display_rect_os{};
    // Region inside the GPU output texture/framebuffer
    math::IntRect viewport_fb{};

    math::Vector2f screenToNDC(const math::Vector2f& point_os) const;

    math::Vector2f screenToFrameBufferPixel(const math::Vector2f& point_os) const;
};

}  // namespace cave
