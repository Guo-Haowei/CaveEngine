#pragma once
#include "cave/runtime/view/ViewRecord.h"

namespace cave {

using math::Vec2f;

Vec2f ViewRecord::screenToNDC(const Vec2f& point_os) const {
    math::Vec2f ndc = ((point_os - display_rect_os.Min()) / display_rect_os.Extent()) * 2.0f - 1.0f;
    ndc.y = -ndc.y;
    return ndc;
}

Vec2f ViewRecord::screenToFrameBufferPixel(const Vec2f& point_os) const {
    // Convert to view space
    Vec2f point = point_os - display_rect_os.Min();
    // Convert to (0, 1) range
    point /= display_rect_os.Extent();
    // to viewport space
    point.x *= viewport_fb.Width();
    point.y *= viewport_fb.Height();
    return point;
}

}  // namespace cave
