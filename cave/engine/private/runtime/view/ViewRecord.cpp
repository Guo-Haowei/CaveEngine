#pragma once
#include "ViewRecord.h"

namespace cave {

using math::Vector2f;

Vector2f ViewRecord::ScreenToNDC(const math::Vector2f& p_point_os) const {
    math::Vector2f ndc = ((p_point_os - display_rect_os.Min()) / display_rect_os.Extent()) * 2.0f - 1.0f;
    ndc.y = -ndc.y;
    return ndc;
}

Vector2f ViewRecord::ScreenToFrameBufferPixel(const math::Vector2f& p_point_os) const {
    // Convert to view space
    Vector2f point = p_point_os - display_rect_os.Min();
    // Convert to (0, 1) range
    point /= display_rect_os.Extent();
    // to viewport space
    point.x *= viewport_fb.Width();
    point.y *= viewport_fb.Height();
    return point;
}

}  // namespace cave
