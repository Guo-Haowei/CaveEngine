// =============================================================================
// File: cave/core/algorithm/Rasterize.h
// =============================================================================
#pragma once

namespace cave {

template<typename Fn>
void ForEachGridLine(int from_x,
                     int from_y,
                     int to_x,
                     int to_y,
                     Fn&& fn) {
    const int dx = std::abs(to_x - from_x);
    const int sx = from_x < to_x ? 1 : -1;

    const int dy = -std::abs(to_y - from_y);
    const int sy = from_y < to_y ? 1 : -1;

    int error = dx + dy;

    while (true) {
        fn(GridCoord{ from_x, from_y });

        if (from_x == to_x && from_y == to_y) {
            break;
        }

        const int twice_error = error * 2;
        if (twice_error >= dy) {
            error += dy;
            from_x += sx;
        }
        if (twice_error <= dx) {
            error += dx;
            from_y += sy;
        }
    }
}
}  // namespace cave
