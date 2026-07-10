#pragma once
#include "GridPaintDefines.h"

namespace cave {

enum class GridPaintMode : uint8_t {
    Brush,
    Line,
    Rect,
};

enum class GridPaintAction : uint8_t {
    Paint,
    Erase,
};

enum class GridPaintEventType : uint8_t {
    Begin,
    Apply,
    End,
    Cancel,
};

enum class GridPaintModifier : uint8_t {
    None,
    Ctrl,
    Shift,
};

struct GridBrush {
    int width = 1;
    int height = 1;

    int anchor_x = 0;
    int anchor_y = 0;

    bool valid() const {
        return width > 0 &&
               height > 0 &&
               anchor_x < width &&
               anchor_y < height;
    }

    static GridBrush Single() {
        return {};
    }

    static GridBrush Centered(int width, int height) {
        DEV_ASSERT(width > 0 && height > 0);
        return GridBrush{
            .width = width,
            .height = height,
            .anchor_x = static_cast<int>(width) / 2,
            .anchor_y = static_cast<int>(height) / 2,
        };
    }
};

using GridPaintPreview = Vector<GridPaintCell>;

struct GridPaintEvent {
    GridPaintEventType type = GridPaintEventType::Apply;
    GridPaintAction action = GridPaintAction::Paint;

    // Only meaningful for Apply.
    const GridPaintPreview* cells = nullptr;
};

template<typename Fn>
void ForEachGridLine(GridCoord from, GridCoord to, Fn&& fn) {
    int x = from.x;
    int y = from.y;

    const int dx = std::abs(to.x - from.x);
    const int sx = from.x < to.x ? 1 : -1;

    const int dy = -std::abs(to.y - from.y);
    const int sy = from.y < to.y ? 1 : -1;

    int error = dx + dy;

    while (true) {
        fn(GridCoord{ x, y });

        if (x == to.x && y == to.y) {
            break;
        }

        const int twice_error = error * 2;
        if (twice_error >= dy) {
            error += dy;
            x += sx;
        }
        if (twice_error <= dx) {
            error += dx;
            y += sy;
        }
    }
}

template<typename Fn>
void ForEachGridRect(GridCoord from, GridCoord to, Fn&& fn) {
    const int min_x = std::min(from.x, to.x);
    const int max_x = std::max(from.x, to.x);
    const int min_y = std::min(from.y, to.y);
    const int max_y = std::max(from.y, to.y);

    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            fn(GridCoord{ x, y });
        }
    }
}

template<typename Fn>
void ForEachBrushCell(GridCoord origin,
                      const GridBrush& brush,
                      Fn&& fn) {
    for (int by = 0; by < brush.height; ++by) {
        for (int bx = 0; bx < brush.width; ++bx) {
            fn(GridCoord{
                   .x = origin.x + bx - brush.anchor_x,
                   .y = origin.y + by - brush.anchor_y,
               },
               bx, by);
        }
    }
}

class GridPaintTool {
public:
    GridPaintMode mode() const { return m_selected_mode; }
    void setMode(GridPaintMode mode) { m_selected_mode = mode; }

    const GridBrush& brush() const { return m_brush; }
    void setBrush(GridBrush brush) {
        if (DEV_VERIFY(brush.valid())) {
            m_brush = brush;
        }
    }

    bool active() const { return m_stroke.active; }

    std::span<const GridPaintCell> preview() const { return m_preview; }

    auto update(const GridPaintInput& input) -> std::span<const GridPaintEvent>;

    void reset();

private:

    struct Stroke {
        bool active = false;

        GridPaintMode mode = GridPaintMode::Brush;
        GridPaintModifier modifier = GridPaintModifier::None;
        GridPaintAction action = GridPaintAction::Paint;

        GridCoord start;
        GridCoord previous;
        GridCoord current;

        GridBrush brush;
    };

    auto resolveMode(const GridPaintInput& input) const
        -> std::pair<GridPaintMode, GridPaintModifier>;

    void beginStroke(GridCoord coord,
                     GridPaintMode mode,
                     GridPaintModifier modifier,
                     GridPaintAction action);

    void updateStroke(GridCoord coord);
    void finishStroke();
    void cancelStroke();

    bool isStrokeModifierHeld(const GridPaintInput& input) const;

    void buildHoverPreview(GridCoord coord,
                           GridPaintMode mode);

    void buildStrokePreview();
    void buildBrushPreview(GridCoord coord,
                           const GridBrush& brush,
                           GridPaintPreview& out);

    void appendBrush(GridCoord coord,
                     const GridBrush& brush,
                     GridPaintPreview& out);

    void emit(GridPaintEventType type,
              GridPaintAction action,
              const GridPaintPreview* cells = nullptr);

private:
    GridPaintMode m_selected_mode = GridPaintMode::Brush;
    GridBrush m_brush = GridBrush::Single();

    Stroke m_stroke;

    GridPaintPreview m_preview;
    GridPaintPreview m_apply_buffer;

    Vector<GridPaintEvent> m_events;
};

}  // namespace cave
