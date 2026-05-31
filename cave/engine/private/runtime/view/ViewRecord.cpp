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
    Vector2f point = p_point_os - display_rect_os.Min();  // view space
    point /= display_rect_os.Extent();                    // x & y are [0, 1]
    point.x *= viewport_fb.Width();                    // to viewport space
    point.y *= viewport_fb.Height();
    return point;
}

}  // namespace cave
