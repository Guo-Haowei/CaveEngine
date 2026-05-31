#pragma once
#include "ViewRecord.h"

namespace cave {

using math::Vector2f;

Vector2f ViewRecord::ScreenToNDC(const math::Vector2f& p_point) const {
    math::Vector2f ndc = ((p_point - rect.Min()) / rect.Extent()) * 2.0f - 1.0f;
    ndc.y = -ndc.y;
    return ndc;
}

Vector2f ViewRecord::ScreenToPixel(const math::Vector2f& p_point) const {
    Vector2f point = p_point - rect.Min();  // view space
    point /= rect.Extent();                 // x & y are [0, 1]
    point.x *= viewport_px.Width();         // to viewport space
    point.y *= viewport_px.Height(); 
    return point;
}

}  // namespace cave
