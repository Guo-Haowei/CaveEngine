#include "GridPaintTool.h"

namespace cave {

auto GridPaintTool::resolveMode(const GridPaintInput& input) const
    -> std::pair<GridPaintMode, GridPaintModifier> {

    if (input.shift) {
        return std::make_pair(GridPaintMode::Line, GridPaintModifier::Shift);
    }

    if (input.ctrl) {
        return std::make_pair(GridPaintMode::Rect, GridPaintModifier::Ctrl);
    }

    return std::make_pair(m_selected_mode, GridPaintModifier::None);
}

void GridPaintTool::emit(GridPaintEventType type,
                         const GridPaintPreview* cells) {
    m_events.push_back(GridPaintEvent{ type, cells });
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
                                GridPaintModifier modifier) {
    m_stroke = {
        .active = true,
        .mode = mode,
        .modifier = modifier,
        .start = coord,
        .previous = coord,
        .current = coord,
        .brush = m_brush,
    };

    buildStrokePreview();

    emit(GridPaintEventType::Begin);

    if (mode == GridPaintMode::Brush) {
        m_apply_buffer = m_preview;
        emit(GridPaintEventType::Apply, &m_apply_buffer);
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

        emit(GridPaintEventType::Apply,
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

    if (m_stroke.mode != GridPaintMode::Brush) {
        buildStrokePreview();

        m_apply_buffer = m_preview;
        emit(GridPaintEventType::Apply, &m_apply_buffer);
    }

    emit(GridPaintEventType::End);

    m_stroke = {};
    m_preview.clear();
}

void GridPaintTool::cancelStroke() {
    if (!m_stroke.active) {
        return;
    }

    m_stroke = {};
    m_preview.clear();

    emit(GridPaintEventType::Cancel);
}

bool GridPaintTool::isStrokeModifierHeld(const GridPaintInput& input) const {
    switch (m_stroke.modifier) {
        case GridPaintModifier::None:
            return true;
        case GridPaintModifier::Ctrl:
            return input.ctrl;
        case GridPaintModifier::Shift:
            return input.shift;
    }
    return false;
}

auto GridPaintTool::update(const GridPaintInput& input) -> std::span<const GridPaintEvent> {
    m_events.clear();

    if (input.alt && input.has_hover) {

        m_apply_buffer.clear();
        m_apply_buffer.push_back({ input.hover });

        if (input.left_down) {
            emit(GridPaintEventType::Fill, &m_apply_buffer);
            return m_events;
        }
    }

    if (m_stroke.active && !(input.has_hover && isStrokeModifierHeld(input))) {
        cancelStroke();
        return m_events;
    }

    if (!m_stroke.active) {
        if (!input.has_hover) {
            m_preview.clear();
            return m_events;
        }

        const auto [mode, modifier] = resolveMode(input);

        if (input.left_pressed) {
            beginStroke(input.hover, mode, modifier);
            return m_events;
        }

        buildHoverPreview(input.hover, mode);
        return m_events;
    }

    // The active mode/action/brush are frozen until release.
    if (input.has_hover) {
        updateStroke(input.hover);
    }

    if (input.left_released) {
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
