#pragma once
#include "cave/core/math/Box.h"

#include "ImageCanvas.h"

namespace cave {

struct AtlasLayout {
    math::Vec2i grid_size{ 1, 1 };
    math::Vec2f image_size_px = math::Vec2f::Zero;

    uint32_t cellCount() const;

    bool valid() const;

    bool contains(math::Vec2i cell) const;
    bool contains(uint32_t index) const;

    math::Vec2f cellSizePx() const;

    uint32_t cellToIndex(math::Vec2i cell) const;
    math::Vec2i indexToCell(uint32_t index) const;

    math::Box2 cellRectPx(math::Vec2i cell) const;
    math::Box2 cellRectPx(uint32_t index) const;

    Option<math::Vec2i> pointToCell(math::Vec2f point_image_px) const;

    Option<uint32_t> pointToIndex(math::Vec2f point_image_px) const;
};

struct AtlasSelection {
    Option<uint32_t> hovered;
    Option<uint32_t> selected;
};

struct AtlasStyle {
    ImU32 grid_color = IM_COL32(120, 130, 145, 150);
    ImU32 hover_color = IM_COL32(235, 240, 250, 255);
    ImU32 selected_color = IM_COL32(95, 155, 235, 255);

    float grid_thickness = 1.0f;
    float hover_thickness = 2.0f;
    float selected_thickness = 3.0f;

    bool draw_grid = true;
};

struct AtlasWidgetDesc {
    const char* id = "##AtlasWidget";

    ImTextureID texture = 0;

    AtlasLayout layout;
    math::Vec2f widget_size = math::Vec2f::Zero;

    AtlasStyle style;

    bool allow_selection = true;
    bool allow_context_menu = true;
    bool show_toolbar = true;
    bool show_checkerboard = true;
};

struct AtlasWidgetResult {
    Option<uint32_t> hovered;
    Option<uint32_t> selected;
    Option<uint32_t> activated;
    Option<uint32_t> context_clicked;

    bool selection_changed = false;
};

class AtlasWidget {
public:
    AtlasWidgetResult draw(const AtlasWidgetDesc& desc,
                           AtlasSelection& selection);

    ImageCanvas& canvas() { return m_canvas; }

    const ImageCanvas& canvas() const { return m_canvas; }

private:
    static ImVec2 imagePointToScreen(const ImageCanvasResult& canvas_result,
                                     float zoom,
                                     math::Vec2f point_image_px);

    static void drawGrid(const AtlasWidgetDesc& desc,
                         const ImageCanvasResult& canvas_result,
                         float zoom);

    static void drawCellOutline(const AtlasWidgetDesc& desc,
                                const ImageCanvasResult& canvas_result,
                                float zoom,
                                uint32_t index,
                                ImU32 color,
                                float thickness);

private:
    ImageCanvas m_canvas;
};

}  // namespace cave