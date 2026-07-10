#include "GridPaintTool.h"

namespace cave {

GridPaintMode GridPaintTool::resolveMode(const GridPaintInput& input) const {
    // Modifiers only override the selected mode while idle.
    if (input.shift) {
        return GridPaintMode::Line;
    }

    if (input.ctrl) {
        return GridPaintMode::Rect;
    }

    return m_selected_mode;
}

void GridPaintTool::emit(GridPaintEventType type,
                         GridPaintAction action,
                         const GridPaintPreview* cells) {
    m_events.push_back(GridPaintEvent{
        .type = type,
        .action = action,
        .cells = cells,
    });
}

void GridPaintTool::appendBrush(GridCoord coord,
                                const GridBrush& brush,
                                GridPaintPreview& out) {
    ForEachBrushCell(coord, brush, [&](GridCoord cell, int32_t brush_x, int32_t brush_y) {
        out.push_back(GridPaintCell{
            .coord = cell,
            .brush_x = brush_x,
            .brush_y = brush_y,
        });
    });
}

void GridPaintTool::buildBrushPreview(GridCoord coord,
                                      const GridBrush& brush,
                                      GridPaintPreview& out) {
    out.clear();
    appendBrush(coord, brush, out);
}

void GridPaintTool::buildStrokePreview() {
    m_preview.clear();

    if (!m_stroke.active) {
        return;
    }

    switch (m_stroke.mode) {
        case GridPaintMode::Brush: {
            appendBrush(
                m_stroke.current,
                m_stroke.brush,
                m_preview);
        } break;

        case GridPaintMode::Line: {
            ForEachGridLine(
                m_stroke.start,
                m_stroke.current,
                [&](GridCoord coord) {
                    appendBrush(coord, m_stroke.brush, m_preview);
                });
        } break;

        case GridPaintMode::Rect: {
            ForEachGridRect(
                m_stroke.start,
                m_stroke.current,
                [&](GridCoord coord) {
                    appendBrush(coord, m_stroke.brush, m_preview);
                });
        } break;
    }
}

void GridPaintTool::buildHoverPreview(GridCoord coord,
                                      GridPaintMode mode) {
    // With no active stroke, line/rect are only one-cell shapes.
    // This still previews the current brush footprint.
    m_preview.clear();

    switch (mode) {
        case GridPaintMode::Brush:
        case GridPaintMode::Line:
        case GridPaintMode::Rect:
            appendBrush(coord, m_brush, m_preview);
            break;
    }
}

void GridPaintTool::beginStroke(GridCoord coord,
                                GridPaintMode mode,
                                GridPaintAction action) {
    m_stroke = {
        .active = true,
        .mode = mode,
        .action = action,
        .start = coord,
        .previous = coord,
        .current = coord,
        .brush = m_brush,
    };

    buildStrokePreview();

    emit(GridPaintEventType::Begin, action);

    if (mode == GridPaintMode::Brush) {
        m_apply_buffer = m_preview;
        emit(GridPaintEventType::Apply, action, &m_apply_buffer);
    }
}

void GridPaintTool::updateStroke(GridCoord coord) {
    if (!m_stroke.active || coord == m_stroke.current) {
        return;
    }

    m_stroke.previous = m_stroke.current;
    m_stroke.current = coord;

    if (m_stroke.mode == GridPaintMode::Brush) {
        // Connect previous and current to avoid gaps when the mouse moves
        // more than one cell in a frame.
        m_apply_buffer.clear();

        ForEachGridLine(
            m_stroke.previous,
            m_stroke.current,
            [&](GridCoord line_cell) {
                appendBrush(
                    line_cell,
                    m_stroke.brush,
                    m_apply_buffer);
            });

        emit(
            GridPaintEventType::Apply,
            m_stroke.action,
            &m_apply_buffer);

        // Ghost only shows the current brush footprint.
        buildBrushPreview(
            m_stroke.current,
            m_stroke.brush,
            m_preview);

        return;
    }

    // Line and rectangle update ghost preview only.
    buildStrokePreview();
}

void GridPaintTool::finishStroke() {
    if (!m_stroke.active) {
        return;
    }

    const GridPaintAction action = m_stroke.action;

    if (m_stroke.mode != GridPaintMode::Brush) {
        buildStrokePreview();

        m_apply_buffer = m_preview;
        emit(GridPaintEventType::Apply, action, &m_apply_buffer);
    }

    emit(GridPaintEventType::End, action);

    m_stroke = {};
    m_preview.clear();
}

void GridPaintTool::cancelStroke() {
    if (!m_stroke.active) {
        return;
    }

    const GridPaintAction action = m_stroke.action;

    m_stroke = {};
    m_preview.clear();

    emit(GridPaintEventType::Cancel, action);
}

auto GridPaintTool::update(const GridPaintInput& input) -> std::span<const GridPaintEvent> {
    m_events.clear();

    const bool cancel_pressed = input.cancel_pressed;
    const bool cursor_outside = m_stroke.active && !input.has_hover;
    if (cancel_pressed || cursor_outside) {
        cancelStroke();
        return m_events;
    }

    if (!m_stroke.active) {
        if (!input.has_hover) {
            m_preview.clear();
            return m_events;
        }

        if (input.left_pressed) {
            beginStroke(
                input.hover,
                resolveMode(input),
                GridPaintAction::Paint);
            return m_events;
        }

        if (input.right_pressed) {
            beginStroke(
                input.hover,
                resolveMode(input),
                GridPaintAction::Erase);
            return m_events;
        }

        buildHoverPreview(
            input.hover,
            resolveMode(input));

        return m_events;
    }

    // The active mode/action/brush are frozen until release.
    if (input.has_hover) {
        updateStroke(input.hover);
    }

    const bool released = m_stroke.action == GridPaintAction::Paint
                              ? input.left_released
                              : input.right_released;

    if (released) {
        finishStroke();
    }

    return m_events;
}

void GridPaintTool::reset() {
    m_stroke = {};
    m_preview.clear();
    m_apply_buffer.clear();
    m_events.clear();
}

}  // namespace cave
