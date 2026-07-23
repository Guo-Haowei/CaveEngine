#pragma once

namespace cave {

struct GridCoord {
    int x;
    int y;

    bool operator==(const GridCoord&) const = default;
};

struct GridPaintCell {
    GridCoord coord;

    // Position inside the brush/pattern.
    int brush_x = 0;
    int brush_y = 0;
};

struct GridPaintInput {
    bool has_hover = false;
    GridCoord hover;

    bool left_pressed = false;
    bool left_down = false;
    bool left_released = false;

    bool right_pressed = false;
    bool right_down = false;
    bool right_released = false;
};

}  // namespace cave