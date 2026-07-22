#include "AtlasWidget.h"

namespace cave {

using namespace ::cave::math;

uint32_t AtlasLayout::cellCount() const {
    if (!valid()) {
        return 0;
    }

    return static_cast<uint32_t>(
        grid_size.x * grid_size.y);
}

bool AtlasLayout::valid() const {
    return grid_size.x > 0 &&
           grid_size.y > 0 &&
           image_size_px.x > 0.0f &&
           image_size_px.y > 0.0f;
}

bool AtlasLayout::contains(Vec2i cell) const {
    return cell.x >= 0 &&
           cell.y >= 0 &&
           cell.x < grid_size.x &&
           cell.y < grid_size.y;
}

bool AtlasLayout::contains(uint32_t index) const {
    return index < cellCount();
}

Vec2f AtlasLayout::cellSizePx() const {
    if (!valid()) {
        return Vec2f::Zero;
    }

    return Vec2f{
        image_size_px.x /
            static_cast<float>(grid_size.x),

        image_size_px.y /
            static_cast<float>(grid_size.y),
    };
}

uint32_t AtlasLayout::cellToIndex(Vec2i cell) const {
    DEV_ASSERT(contains(cell));

    return static_cast<uint32_t>(
        cell.y * grid_size.x + cell.x);
}

Vec2i AtlasLayout::indexToCell(uint32_t index) const {
    DEV_ASSERT(contains(index));

    return Vec2i{
        static_cast<int>(index %
                         static_cast<uint32_t>(grid_size.x)),

        static_cast<int>(index /
                         static_cast<uint32_t>(grid_size.x)),
    };
}

Box2 AtlasLayout::cellRectPx(Vec2i cell) const {
    DEV_ASSERT(contains(cell));

    const Vec2f cell_size =
        cellSizePx();

    const Vec2f min{
        static_cast<float>(cell.x) * cell_size.x,
        static_cast<float>(cell.y) * cell_size.y,
    };

    return Box2{
        min,
        min + cell_size,
    };
}

Box2 AtlasLayout::cellRectPx(uint32_t index) const {
    return cellRectPx(indexToCell(index));
}

Option<Vec2i> AtlasLayout::pointToCell(
    Vec2f point_image_px) const {

    if (!valid()) {
        return None();
    }

    if (point_image_px.x < 0.0f ||
        point_image_px.y < 0.0f ||
        point_image_px.x >= image_size_px.x ||
        point_image_px.y >= image_size_px.y) {
        return None();
    }

    const Vec2f cell_size =
        cellSizePx();

    const Vec2i cell{
        static_cast<int>(
            std::floor(
                point_image_px.x / cell_size.x)),

        static_cast<int>(
            std::floor(
                point_image_px.y / cell_size.y)),
    };

    if (!contains(cell)) {
        return None();
    }

    return Some(cell);
}

Option<uint32_t> AtlasLayout::pointToIndex(
    Vec2f point_image_px) const {

    auto cell =
        pointToCell(point_image_px);

    if (!cell) {
        return None();
    }

    return Some(
        cellToIndex(cell.unwrap_unchecked()));
}

ImVec2 AtlasWidget::imagePointToScreen(
    const ImageCanvasResult& canvas_result,
    float zoom,
    Vec2f point_image_px) {

    return ImVec2{
        canvas_result.image_min_ss.x +
            point_image_px.x * zoom,

        canvas_result.image_min_ss.y +
            point_image_px.y * zoom,
    };
}

void AtlasWidget::drawGrid(
    const AtlasWidgetDesc& desc,
    const ImageCanvasResult& canvas_result,
    float zoom) {

    if (!desc.style.draw_grid ||
        !canvas_result.draw_list ||
        !desc.layout.valid()) {
        return;
    }

    ImDrawList& draw_list =
        *canvas_result.draw_list;

    const Vec2f cell_size =
        desc.layout.cellSizePx();

    for (int x = 0;
         x <= desc.layout.grid_size.x;
         ++x) {
        const float image_x =
            static_cast<float>(x) *
            cell_size.x;

        const ImVec2 from =
            imagePointToScreen(
                canvas_result,
                zoom,
                Vec2f{ image_x, 0.0f });

        const ImVec2 to =
            imagePointToScreen(
                canvas_result,
                zoom,
                Vec2f{
                    image_x,
                    desc.layout.image_size_px.y,
                });

        draw_list.AddLine(
            from,
            to,
            desc.style.grid_color,
            desc.style.grid_thickness);
    }

    for (int y = 0;
         y <= desc.layout.grid_size.y;
         ++y) {
        const float image_y =
            static_cast<float>(y) *
            cell_size.y;

        const ImVec2 from =
            imagePointToScreen(
                canvas_result,
                zoom,
                Vec2f{ 0.0f, image_y });

        const ImVec2 to =
            imagePointToScreen(
                canvas_result,
                zoom,
                Vec2f{
                    desc.layout.image_size_px.x,
                    image_y,
                });

        draw_list.AddLine(
            from,
            to,
            desc.style.grid_color,
            desc.style.grid_thickness);
    }
}

void AtlasWidget::drawCellOutline(
    const AtlasWidgetDesc& desc,
    const ImageCanvasResult& canvas_result,
    float zoom,
    uint32_t index,
    ImU32 color,
    float thickness) {

    if (!canvas_result.draw_list ||
        !desc.layout.contains(index)) {
        return;
    }

    const Box2 rect =
        desc.layout.cellRectPx(index);

    const ImVec2 min =
        imagePointToScreen(
            canvas_result,
            zoom,
            rect.min());

    const ImVec2 max =
        imagePointToScreen(
            canvas_result,
            zoom,
            rect.max());

    canvas_result.draw_list->AddRect(
        min,
        max,
        color,
        0.0f,
        0,
        thickness);
}

AtlasWidgetResult AtlasWidget::draw(
    const AtlasWidgetDesc& desc,
    AtlasSelection& selection) {

    AtlasWidgetResult result;

    ImageCanvasDesc canvas_desc;
    canvas_desc.id = desc.id;
    canvas_desc.texture = desc.texture;
    canvas_desc.image_size_px =
        desc.layout.image_size_px;
    canvas_desc.widget_size =
        desc.widget_size;
    canvas_desc.show_toolbar =
        desc.show_toolbar;
    canvas_desc.show_checkerboard =
        desc.show_checkerboard;

    const ImageCanvasResult canvas_result =
        m_canvas.draw(canvas_desc);

    Option<uint32_t> hovered;

    if (canvas_result.mouse_image_px) {
        hovered = desc.layout.pointToIndex(
            canvas_result.mouse_image_px
                .unwrap_unchecked());
    }

    selection.hovered = hovered;
    result.hovered = hovered;

    if (hovered) {
        const uint32_t index =
            hovered.unwrap_unchecked();

        if (desc.allow_selection &&
            canvas_result.left_clicked) {
            if (selection.selected != Some(index)) {
                selection.selected = Some(index);
                result.selection_changed = true;
            }

            result.selected = Some(index);
        }

        if (desc.allow_selection &&
            canvas_result.left_double_clicked) {
            selection.selected = Some(index);
            result.activated = Some(index);
        }

        if (desc.allow_context_menu &&
            canvas_result.right_clicked) {
            selection.selected = Some(index);
            result.context_clicked = Some(index);
        }
    }

    drawGrid(
        desc,
        canvas_result,
        m_canvas.zoom());

    if (selection.hovered) {
        drawCellOutline(
            desc,
            canvas_result,
            m_canvas.zoom(),
            selection.hovered.unwrap_unchecked(),
            desc.style.hover_color,
            desc.style.hover_thickness);
    }

    if (selection.selected) {
        drawCellOutline(
            desc,
            canvas_result,
            m_canvas.zoom(),
            selection.selected.unwrap_unchecked(),
            desc.style.selected_color,
            desc.style.selected_thickness);
    }

    return result;
}

}  // namespace cave