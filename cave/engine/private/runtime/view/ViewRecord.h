#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/core/math/Box.h"
#include "cave/core/math/Vector.h"

namespace cave {

struct ViewRecord {
    ViewId view_id;

    std::string debug_name;

    math::FloatRect rect{};  // view rect in screen space
    math::IntRect viewport_px{};

    //// Optional: last submitted output.
    // GpuTextureId output{};

    uint64_t last_submitted_frame = 0;
    uint64_t last_visible_frame = 0;

    math::Vector2f ScreenToNDC(const math::Vector2f& p_cursor_screen) const {
        math::Vector2f ndc = ((p_cursor_screen - rect.Min()) / rect.Extent()) * 2.0f - 1.0f;
        ndc.y = -ndc.y;
        return ndc;
    }
};

}  // namespace cave
