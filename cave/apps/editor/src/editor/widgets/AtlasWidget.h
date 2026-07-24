#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>

#include <imgui.h>
#include <imgui_internal.h>

#include "cave/core/Option.h"
#include "cave/core/math/Box.h"
#include "cave/core/math/Vec.h"

namespace cave {

using namespace ::cave::math;

// =============================================================================
// ImageCanvas
// =============================================================================

struct ImageCanvasInput {
    // Pointer position in ImGui screen coordinates.
    ImVec2 pointer_ss{};

    // Whether pointer_ss contains a meaningful value.
    bool pointer_valid = false;

    // Generic zoom request expressed in zoom steps.
    //
    // +1 means multiply zoom by zoom_step.
    // -1 means divide zoom by zoom_step.
    //
    // This may come from a mouse wheel, keyboard, controller, or anything else.
    float zoom_steps = 0.0f;

    bool left_pressed = false;
    bool left_released = false;
    bool left_double_clicked = false;

    bool right_pressed = false;
    bool right_released = false;
};

struct ImageCanvasDesc {
    const char* id = "##ImageCanvas";

    ImTextureID texture = 0;

    // Native image size in pixels.
    Vec2f image_size_px = Vec2f::Zero;

    // Size of the complete widget, including the toolbar.
    // Zero uses the available ImGui content region.
    Vec2f widget_size = Vec2f::Zero;

    float min_zoom = 0.125f;
    float max_zoom = 16.0f;
    float zoom_step = 1.25f;

    bool show_toolbar = true;
    bool show_checkerboard = false;

    // Optional externally collected input.
    //
    // ImageCanvas does not read pointer buttons or mouse wheel directly.
    // The toolbar still uses normal ImGui buttons.
    const ImageCanvasInput* input = nullptr;
};

struct ImageCanvasTransform {
    Vec2f image_size_px = Vec2f::Zero;

    ImVec2 image_min_ss{};
    ImVec2 image_max_ss{};

    float zoom = 1.0f;

    bool valid() const {
        return zoom > 0.0f &&
               image_size_px.x > 0.0f &&
               image_size_px.y > 0.0f;
    }

    ImVec2 imageToScreen(Vec2f point_image_px) const {
        return ImVec2{
            image_min_ss.x + point_image_px.x * zoom,
            image_min_ss.y + point_image_px.y * zoom,
        };
    }

    Vec2f screenToImageUnchecked(ImVec2 point_ss) const {
        return Vec2f{
            (point_ss.x - image_min_ss.x) / zoom,
            (point_ss.y - image_min_ss.y) / zoom,
        };
    }

    Option<Vec2f> screenToImage(ImVec2 point_ss) const {
        if (!valid()) {
            return None();
        }

        if (point_ss.x < image_min_ss.x ||
            point_ss.y < image_min_ss.y ||
            point_ss.x >= image_max_ss.x ||
            point_ss.y >= image_max_ss.y) {
            return None();
        }

        return Some(screenToImageUnchecked(point_ss));
    }

    bool containsScreenPoint(ImVec2 point_ss) const {
        return point_ss.x >= image_min_ss.x &&
               point_ss.y >= image_min_ss.y &&
               point_ss.x < image_max_ss.x &&
               point_ss.y < image_max_ss.y;
    }
};

struct ImageCanvasResult {
    bool visible = false;
    bool hovered = false;

    bool left_pressed = false;
    bool left_released = false;
    bool left_double_clicked = false;

    bool right_pressed = false;
    bool right_released = false;

    Option<Vec2f> pointer_image_px;

    ImageCanvasTransform transform;

    ImDrawList* draw_list = nullptr;
};

class ImageCanvas {
public:
    ImageCanvasResult draw(const ImageCanvasDesc& desc) {
        ImageCanvasResult result;

        ImGui::PushID(desc.id);

        Vec2f widget_size = desc.widget_size;

        if (widget_size.x <= 0.0f ||
            widget_size.y <= 0.0f) {
            const ImVec2 available =
                ImGui::GetContentRegionAvail();

            if (widget_size.x <= 0.0f) {
                widget_size.x = available.x;
            }

            if (widget_size.y <= 0.0f) {
                widget_size.y = available.y;
            }
        }

        widget_size.x = std::max(widget_size.x, 1.0f);
        widget_size.y = std::max(widget_size.y, 1.0f);

        float toolbar_height = 0.0f;

        if (desc.show_toolbar) {
            toolbar_height =
                ImGui::GetFrameHeightWithSpacing();

            const Vec2f fit_area{
                widget_size.x,
                std::max(
                    widget_size.y - toolbar_height,
                    1.0f),
            };

            drawToolbar(desc, fit_area);
        }

        const Vec2f child_size{
            widget_size.x,
            std::max(
                widget_size.y - toolbar_height,
                1.0f),
        };

        const ImGuiWindowFlags child_flags =
            ImGuiWindowFlags_HorizontalScrollbar |
            ImGuiWindowFlags_AlwaysVerticalScrollbar;

        const bool child_visible =
            ImGui::BeginChild(
                "##ImageCanvasScroll",
                toImVec2(child_size),
                ImGuiChildFlags_Borders,
                child_flags);

        result.visible = child_visible;

        if (!child_visible) {
            ImGui::EndChild();
            ImGui::PopID();
            return result;
        }

        m_last_image_size_px =
            desc.image_size_px;

        const Vec2f content_size =
            toVec2f(ImGui::GetContentRegionAvail());

        if (m_fit_requested) {
            applyFit(desc, content_size);
        }

        m_zoom = clampZoom(desc, m_zoom);

        const Vec2f displayed_size{
            math::max(
                desc.image_size_px.x * m_zoom,
                1.0f),

            math::max(
                desc.image_size_px.y * m_zoom,
                1.0f),
        };

        // Center the image when it is smaller than the viewport.
        const Vec2f padding{
            math::max(
                (content_size.x - displayed_size.x) * 0.5f,
                0.0f),

            math::max(
                (content_size.y - displayed_size.y) * 0.5f,
                0.0f),
        };

        if (padding.x > 0.0f) {
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + padding.x);
        }

        if (padding.y > 0.0f) {
            ImGui::SetCursorPosY(
                ImGui::GetCursorPosY() + padding.y);
        }

        const ImVec2 image_min_ss =
            ImGui::GetCursorScreenPos();

        const ImVec2 image_max_ss{
            image_min_ss.x + displayed_size.x,
            image_min_ss.y + displayed_size.y,
        };

        ImDrawList* draw_list =
            ImGui::GetWindowDrawList();

        if (desc.show_checkerboard) {
            drawCheckerboard(
                *draw_list,
                image_min_ss,
                image_max_ss,
                8.0f * m_zoom);
        }

        if (desc.texture != 0) {
            ImGui::Image(
                desc.texture,
                toImVec2(displayed_size));
        } else {
            ImGui::InvisibleButton(
                "##MissingImage",
                toImVec2(displayed_size));
        }

        result.transform.image_size_px =
            desc.image_size_px;

        result.transform.image_min_ss =
            image_min_ss;

        result.transform.image_max_ss =
            image_max_ss;

        result.transform.zoom =
            m_zoom;

        result.draw_list =
            draw_list;

        m_last_transform =
            result.transform;

        handleExternalInput(
            desc,
            result,
            content_size);

        applyPendingZoom(
            desc,
            result.transform,
            content_size);

        ImGui::EndChild();
        ImGui::PopID();

        return result;
    }

    float zoom() const {
        return m_zoom;
    }

    void setZoom(float zoom,
                 float min_zoom = 0.125f,
                 float max_zoom = 16.0f) {
        m_zoom = math::clamp(
            zoom,
            min_zoom,
            max_zoom);

        m_fit_requested = false;
    }

    void resetZoom() {
        m_zoom = 1.0f;
        m_fit_requested = false;
    }

    void requestFit() {
        m_fit_requested = true;
    }

    // Queue a zoom operation around a specific screen-space point.
    //
    // The request is applied during draw(), while the scroll child is active.
    void requestZoomAt(ImVec2 anchor_ss,
                       float steps) {
        if (steps == 0.0f) {
            return;
        }

        m_pending_zoom.anchor_ss =
            anchor_ss;

        m_pending_zoom.steps +=
            steps;

        m_pending_zoom.use_viewport_center =
            false;
    }

    // Queue zoom around the center of the image viewport.
    //
    // Used by the existing + and - toolbar buttons.
    void requestZoomAtViewportCenter(float steps) {
        if (steps == 0.0f) {
            return;
        }

        m_pending_zoom.steps +=
            steps;

        m_pending_zoom.use_viewport_center =
            true;
    }

    Vec2f displayedImageSize() const {
        return m_last_image_size_px * m_zoom;
    }

    ImVec2 imageToScreen(Vec2f point_px) const {
        return m_last_transform.imageToScreen(point_px);
    }

    Option<Vec2f> screenToImage(ImVec2 point_ss) const {
        return m_last_transform.screenToImage(point_ss);
    }

private:
    struct PendingZoom {
        ImVec2 anchor_ss{};

        float steps = 0.0f;

        bool use_viewport_center = false;

        void clear() {
            anchor_ss = ImVec2{};
            steps = 0.0f;
            use_viewport_center = false;
        }
    };

private:
    static ImVec2 toImVec2(Vec2f value) {
        return ImVec2{
            value.x,
            value.y,
        };
    }

    static Vec2f toVec2f(ImVec2 value) {
        return Vec2f{
            value.x,
            value.y,
        };
    }

    static float clampZoom(
        const ImageCanvasDesc& desc,
        float zoom) {

        return math::clamp(
            zoom,
            desc.min_zoom,
            desc.max_zoom);
    }

    void drawToolbar(
        const ImageCanvasDesc& desc,
        Vec2f available_size) {

        const float button_width =
            ImGui::GetFrameHeight();

        if (ImGui::Button(
                "-",
                ImVec2{ button_width, 0.0f })) {
            requestZoomAtViewportCenter(-1.0f);
        }

        ImGui::SameLine();

        char zoom_text[32];

        std::snprintf(
            zoom_text,
            sizeof(zoom_text),
            "%.0f%%",
            m_zoom * 100.0f);

        const float zoom_text_width =
            std::max(
                64.0f,
                ImGui::CalcTextSize(zoom_text).x + 20.0f);

        ImGui::SetNextItemWidth(
            zoom_text_width);

        if (ImGui::Button(
                zoom_text,
                ImVec2{ zoom_text_width, 0.0f })) {
            resetZoom();
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
                "Reset zoom to 100%%");
        }

        ImGui::SameLine();

        if (ImGui::Button(
                "+",
                ImVec2{ button_width, 0.0f })) {
            requestZoomAtViewportCenter(1.0f);
        }

        ImGui::SameLine();

        if (ImGui::Button("1:1")) {
            resetZoom();
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
                "Actual image size");
        }

        ImGui::SameLine();

        if (ImGui::Button("Fit")) {
            m_fit_requested = true;
            applyFit(
                desc,
                available_size);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
                "Fit image to available space");
        }
    }

    void applyFit(
        const ImageCanvasDesc& desc,
        Vec2f available_size) {

        if (desc.image_size_px.x <= 0.0f ||
            desc.image_size_px.y <= 0.0f ||
            available_size.x <= 0.0f ||
            available_size.y <= 0.0f) {
            return;
        }

        const float zoom_x =
            available_size.x /
            desc.image_size_px.x;

        const float zoom_y =
            available_size.y /
            desc.image_size_px.y;

        m_zoom = clampZoom(
            desc,
            math::min(zoom_x, zoom_y));

        m_fit_requested = false;
    }

    void handleExternalInput(
        const ImageCanvasDesc& desc,
        ImageCanvasResult& result,
        Vec2f content_size) {

        if (!desc.input ||
            !desc.input->pointer_valid) {
            return;
        }

        const ImageCanvasInput& input =
            *desc.input;

        result.hovered =
            result.transform.containsScreenPoint(
                input.pointer_ss);

        if (result.hovered) {
            result.pointer_image_px =
                Some(result.transform.screenToImageUnchecked(input.pointer_ss));
            result.left_pressed = input.left_pressed;
            result.left_released = input.left_released; 
            result.left_double_clicked = input.left_double_clicked;
            result.right_pressed = input.right_pressed;
            result.right_released = input.right_released;
        }

        if (input.zoom_steps != 0.0f) {
            requestZoomAt(
                input.pointer_ss,
                input.zoom_steps);
        }

        static_cast<void>(content_size);
    }

    void applyPendingZoom(
        const ImageCanvasDesc& desc,
        const ImageCanvasTransform& transform,
        Vec2f content_size) {

        if (m_pending_zoom.steps == 0.0f ||
            !transform.valid()) {
            return;
        }

        ImGuiWindow* window =
            ImGui::GetCurrentWindow();

        if (!window) {
            m_pending_zoom.clear();
            return;
        }

        ImVec2 anchor_ss =
            m_pending_zoom.anchor_ss;

        if (m_pending_zoom.use_viewport_center) {
            const ImVec2 content_min_ss =
                ImGui::GetWindowPos() +
                ImGui::GetWindowContentRegionMin();

            anchor_ss = ImVec2{
                content_min_ss.x +
                    content_size.x * 0.5f,

                content_min_ss.y +
                    content_size.y * 0.5f,
            };
        }

        // An arbitrary anchor is allowed, but clamping it to the visible image
        // produces more predictable behavior for toolbar and keyboard zoom.
        anchor_ss.x = math::clamp(
            anchor_ss.x,
            transform.image_min_ss.x,
            transform.image_max_ss.x);

        anchor_ss.y = math::clamp(
            anchor_ss.y,
            transform.image_min_ss.y,
            transform.image_max_ss.y);

        const float old_zoom =
            m_zoom;

        const float factor =
            std::pow(
                desc.zoom_step,
                m_pending_zoom.steps);

        const float new_zoom =
            clampZoom(
                desc,
                old_zoom * factor);

        if (math::abs(
                new_zoom - old_zoom) <
            0.000001f) {
            m_pending_zoom.clear();
            return;
        }

        // Image-space point currently beneath the anchor.
        const Vec2f anchor_image_px{
            (anchor_ss.x -
             transform.image_min_ss.x) /
                old_zoom,

            (anchor_ss.y -
             transform.image_min_ss.y) /
                old_zoom,
        };

        // How much farther from the image origin that same image-space point
        // will be after zooming.
        const Vec2f old_anchor_offset{
            anchor_image_px.x * old_zoom,
            anchor_image_px.y * old_zoom,
        };

        const Vec2f new_anchor_offset{
            anchor_image_px.x * new_zoom,
            anchor_image_px.y * new_zoom,
        };

        const Vec2f scroll_delta{
            new_anchor_offset.x -
                old_anchor_offset.x,

            new_anchor_offset.y -
                old_anchor_offset.y,
        };

        m_zoom =
            new_zoom;

        m_fit_requested =
            false;

        ImGui::SetScrollX(
            math::max(
                ImGui::GetScrollX() +
                    scroll_delta.x,
                0.0f));

        ImGui::SetScrollY(
            math::max(
                ImGui::GetScrollY() +
                    scroll_delta.y,
                0.0f));

        m_pending_zoom.clear();
    }

    static void drawCheckerboard(
        ImDrawList& draw_list,
        ImVec2 min,
        ImVec2 max,
        float square_size) {

        const ImU32 light =
            IM_COL32(204, 204, 204, 255);

        const ImU32 dark =
            IM_COL32(136, 136, 136, 255);

        int row = 0;

        for (float y = min.y;
             y < max.y;
             y += square_size, ++row) {
            int column = 0;

            for (float x = min.x;
                 x < max.x;
                 x += square_size, ++column) {
                const ImVec2 cell_min{
                    x,
                    y,
                };

                const ImVec2 cell_max{
                    std::min(
                        x + square_size,
                        max.x),

                    std::min(
                        y + square_size,
                        max.y),
                };

                const bool is_light =
                    ((row + column) & 1) == 0;

                draw_list.AddRectFilled(
                    cell_min,
                    cell_max,
                    is_light ? light : dark);
            }
        }
    }

private:
    float m_zoom = 1.0f;

    bool m_fit_requested = true;

    PendingZoom m_pending_zoom;

    Vec2f m_last_image_size_px =
        Vec2f::Zero;

    ImageCanvasTransform m_last_transform;
};

// =============================================================================
// Atlas layout
// =============================================================================

struct AtlasLayout {
    Vec2i grid_size{ 1, 1 };

    Vec2f image_size_px =
        Vec2f::Zero;

    uint32_t cellCount() const {
        if (!valid()) {
            return 0;
        }

        return static_cast<uint32_t>(
            grid_size.x * grid_size.y);
    }

    bool valid() const {
        return grid_size.x > 0 &&
               grid_size.y > 0 &&
               image_size_px.x > 0.0f &&
               image_size_px.y > 0.0f;
    }

    bool contains(Vec2i cell) const {
        return cell.x >= 0 &&
               cell.y >= 0 &&
               cell.x < grid_size.x &&
               cell.y < grid_size.y;
    }

    bool contains(uint32_t index) const {
        return index < cellCount();
    }

    Vec2f cellSizePx() const {
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

    uint32_t cellToIndex(Vec2i cell) const {
        DEV_ASSERT(contains(cell));

        return static_cast<uint32_t>(
            cell.y * grid_size.x +
            cell.x);
    }

    Vec2i indexToCell(uint32_t index) const {
        DEV_ASSERT(contains(index));

        return Vec2i{
            static_cast<int>(
                index %
                static_cast<uint32_t>(
                    grid_size.x)),

            static_cast<int>(
                index /
                static_cast<uint32_t>(
                    grid_size.x)),
        };
    }

    Box2 cellRectPx(Vec2i cell) const {
        DEV_ASSERT(contains(cell));

        const Vec2f cell_size =
            cellSizePx();

        const Vec2f min{
            static_cast<float>(cell.x) *
                cell_size.x,

            static_cast<float>(cell.y) *
                cell_size.y,
        };

        return Box2{
            min,
            min + cell_size,
        };
    }

    Box2 cellRectPx(uint32_t index) const {
        return cellRectPx(
            indexToCell(index));
    }

    Option<Vec2i> pointToCell(
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
                    point_image_px.x /
                    cell_size.x)),

            static_cast<int>(
                std::floor(
                    point_image_px.y /
                    cell_size.y)),
        };

        if (!contains(cell)) {
            return None();
        }

        return Some(cell);
    }

    Option<uint32_t> pointToIndex(
        Vec2f point_image_px) const {

        const Option<Vec2i> cell =
            pointToCell(point_image_px);

        if (!cell) {
            return None();
        }

        return Some(
            cellToIndex(
                cell.unwrap_unchecked()));
    }
};

// =============================================================================
// Atlas input/output
// =============================================================================

struct AtlasHit {
    uint32_t index = 0;

    Vec2i cell =
        Vec2i::Zero;

    // Position relative to the entire image.
    Vec2f image_position_px =
        Vec2f::Zero;

    // Position relative to the cell's top-left corner.
    Vec2f cell_position_px =
        Vec2f::Zero;

    // Normalized position within the cell.
    Vec2f cell_uv =
        Vec2f::Zero;

    ImVec2 screen_position{};

    Vec2i subCell(
        int columns,
        int rows) const {

        DEV_ASSERT(columns > 0);
        DEV_ASSERT(rows > 0);

        Vec2i sub_cell{
            static_cast<int>(
                cell_uv.x *
                static_cast<float>(columns)),

            static_cast<int>(
                cell_uv.y *
                static_cast<float>(rows)),
        };

        sub_cell.x =
            math::clamp(
                sub_cell.x,
                0,
                columns - 1);

        sub_cell.y =
            math::clamp(
                sub_cell.y,
                0,
                rows - 1);

        return sub_cell;
    }

    Vec2i mask3x3Cell() const {
        return subCell(3, 3);
    }

    uint32_t mask3x3Bit() const {
        const Vec2i sub_cell =
            mask3x3Cell();

        return static_cast<uint32_t>(
            sub_cell.y * 3 +
            sub_cell.x);
    }
};

struct AtlasPointerState {
    Option<AtlasHit> hovered;

    Option<AtlasHit> left_pressed;
    Option<AtlasHit> left_released;
    Option<AtlasHit> left_double_clicked;

    Option<AtlasHit> right_pressed;
    Option<AtlasHit> right_released;

    // Cell where the active left-button stroke began.
    Option<AtlasHit> stroke_origin;

    // Current pointer in image coordinates while the stroke is active.
    //
    // This may be outside the image bounds.
    Option<Vec2f> captured_pointer_image_px;

    bool stroke_active = false;
    bool stroke_started = false;
    bool stroke_ended = false;
};

struct AtlasStyle {
    ImU32 grid_color =
        IM_COL32(120, 130, 145, 150);

    ImU32 hover_color =
        IM_COL32(235, 240, 250, 255);

    float grid_thickness =
        1.0f;

    float hover_thickness =
        2.0f;

    bool draw_grid =
        true;

    bool draw_hover =
        true;
};

struct AtlasWidgetDesc {
    const char* id =
        "##AtlasWidget";

    ImTextureID texture = 0;

    AtlasLayout layout;

    Vec2f widget_size =
        Vec2f::Zero;

    AtlasStyle style;

    bool show_toolbar =
        true;

    bool show_checkerboard =
        true;

    // Externally gathered input.
    const ImageCanvasInput* input =
        nullptr;
};

struct AtlasWidgetResult {
    bool visible = false;
    bool hovered = false;

    AtlasPointerState pointer;

    ImageCanvasTransform transform;

    ImDrawList* draw_list = nullptr;

    ImVec2 imageToScreen(
        Vec2f point_image_px) const {

        return transform.imageToScreen(
            point_image_px);
    }

    Option<Vec2f> screenToImage(
        ImVec2 point_ss) const {

        return transform.screenToImage(
            point_ss);
    }

    ImVec2 cellMinToScreen(
        const AtlasLayout& layout,
        uint32_t index) const {

        const Box2 rect =
            layout.cellRectPx(index);

        return imageToScreen(
            rect.min());
    }

    ImVec2 cellMaxToScreen(
        const AtlasLayout& layout,
        uint32_t index) const {

        const Box2 rect =
            layout.cellRectPx(index);

        return imageToScreen(
            rect.max());
    }
};

// =============================================================================
// AtlasWidget
// =============================================================================

class AtlasWidget {
public:
    AtlasWidgetResult draw(
        const AtlasWidgetDesc& desc) {

        AtlasWidgetResult result;

        ImageCanvasDesc canvas_desc;

        canvas_desc.id =
            desc.id;

        canvas_desc.texture =
            desc.texture;

        canvas_desc.image_size_px =
            desc.layout.image_size_px;

        canvas_desc.widget_size =
            desc.widget_size;

        canvas_desc.show_toolbar =
            desc.show_toolbar;

        canvas_desc.show_checkerboard =
            desc.show_checkerboard;

        canvas_desc.input =
            desc.input;

        const ImageCanvasResult canvas_result =
            m_canvas.draw(canvas_desc);

        result.visible =
            canvas_result.visible;

        result.hovered =
            canvas_result.hovered;

        result.transform =
            canvas_result.transform;

        result.draw_list =
            canvas_result.draw_list;

        Option<AtlasHit> current_hit;

        if (desc.input && desc.input->pointer_valid && canvas_result.pointer_image_px) {
            current_hit = buildHit(desc.layout,
                                   canvas_result.pointer_image_px.unwrap_unchecked(),
                                   desc.input->pointer_ss);
        }

        result.pointer.hovered =
            current_hit;

        handleStroke(
            desc,
            canvas_result,
            current_hit,
            result.pointer);

        if (desc.style.draw_grid) {
            drawGrid(
                desc,
                result);
        }

        if (desc.style.draw_hover &&
            current_hit) {
            drawCellOutline(
                desc,
                result,
                current_hit.unwrap_unchecked().index,
                desc.style.hover_color,
                desc.style.hover_thickness);
        }

        return result;
    }

    ImageCanvas& canvas() {
        return m_canvas;
    }

    const ImageCanvas& canvas() const {
        return m_canvas;
    }

    bool strokeActive() const {
        return m_stroke_active;
    }

    void cancelStroke() {
        m_stroke_active = false;
        m_stroke_origin = None();
    }

private:
    static Option<AtlasHit> buildHit(
        const AtlasLayout& layout,
        Vec2f image_position_px,
        ImVec2 screen_position) {

        const Option<Vec2i> cell =
            layout.pointToCell(
                image_position_px);

        if (!cell) {
            return None();
        }

        const Vec2i cell_value =
            cell.unwrap_unchecked();

        const Box2 cell_rect =
            layout.cellRectPx(
                cell_value);

        const Vec2f cell_size =
            layout.cellSizePx();

        const Vec2f cell_position_px{
            image_position_px.x -
                cell_rect.min().x,

            image_position_px.y -
                cell_rect.min().y,
        };

        Vec2f cell_uv{
            cell_size.x > 0.0f
                ? cell_position_px.x /
                      cell_size.x
                : 0.0f,

            cell_size.y > 0.0f
                ? cell_position_px.y /
                      cell_size.y
                : 0.0f,
        };

        const float max_uv =
            std::nextafter(
                1.0f,
                0.0f);

        cell_uv.x =
            math::clamp(
                cell_uv.x,
                0.0f,
                max_uv);

        cell_uv.y =
            math::clamp(
                cell_uv.y,
                0.0f,
                max_uv);

        AtlasHit hit;

        hit.index =
            layout.cellToIndex(
                cell_value);

        hit.cell =
            cell_value;

        hit.image_position_px =
            image_position_px;

        hit.cell_position_px =
            cell_position_px;

        hit.cell_uv =
            cell_uv;

        hit.screen_position =
            screen_position;

        return Some(hit);
    }

    void handleStroke(
        const AtlasWidgetDesc& desc,
        const ImageCanvasResult& canvas_result,
        Option<AtlasHit> current_hit,
        AtlasPointerState& pointer) {

        if (!desc.input) {
            return;
        }

        const ImageCanvasInput& input =
            *desc.input;

        if (canvas_result.left_pressed &&
            current_hit) {
            m_stroke_active = true;

            m_stroke_origin =
                current_hit;

            pointer.left_pressed =
                current_hit;

            pointer.stroke_started =
                true;
        }

        if (canvas_result.left_double_clicked &&
            current_hit) {
            pointer.left_double_clicked =
                current_hit;
        }

        if (canvas_result.right_pressed &&
            current_hit) {
            pointer.right_pressed =
                current_hit;
        }

        if (canvas_result.right_released &&
            current_hit) {
            pointer.right_released =
                current_hit;
        }

        if (m_stroke_active) {
            pointer.stroke_active =
                true;

            pointer.stroke_origin =
                m_stroke_origin;

            if (input.pointer_valid) {
                pointer.captured_pointer_image_px =
                    Some(
                        canvas_result.transform
                            .screenToImageUnchecked(
                                input.pointer_ss));
            }

            if (input.left_released) {
                pointer.left_released =
                    current_hit;

                pointer.stroke_ended =
                    true;

                m_stroke_active =
                    false;

                m_stroke_origin =
                    None();
            }
        }
    }

    static void drawGrid(
        const AtlasWidgetDesc& desc,
        const AtlasWidgetResult& result) {

        if (!result.draw_list ||
            !desc.layout.valid()) {
            return;
        }

        const Vec2f cell_size =
            desc.layout.cellSizePx();

        for (int x = 0;
             x <= desc.layout.grid_size.x;
             ++x) {
            const float image_x =
                static_cast<float>(x) *
                cell_size.x;

            const ImVec2 from =
                result.imageToScreen(
                    Vec2f{
                        image_x,
                        0.0f,
                    });

            const ImVec2 to =
                result.imageToScreen(
                    Vec2f{
                        image_x,
                        desc.layout.image_size_px.y,
                    });

            result.draw_list->AddLine(
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
                result.imageToScreen(
                    Vec2f{
                        0.0f,
                        image_y,
                    });

            const ImVec2 to =
                result.imageToScreen(
                    Vec2f{
                        desc.layout.image_size_px.x,
                        image_y,
                    });

            result.draw_list->AddLine(
                from,
                to,
                desc.style.grid_color,
                desc.style.grid_thickness);
        }
    }

    static void drawCellOutline(
        const AtlasWidgetDesc& desc,
        const AtlasWidgetResult& result,
        uint32_t index,
        ImU32 color,
        float thickness) {

        if (!result.draw_list ||
            !desc.layout.contains(index)) {
            return;
        }

        const Box2 rect =
            desc.layout.cellRectPx(index);

        const ImVec2 min =
            result.imageToScreen(
                rect.min());

        const ImVec2 max =
            result.imageToScreen(
                rect.max());

        result.draw_list->AddRect(
            min,
            max,
            color,
            0.0f,
            0,
            thickness);
    }

private:
    ImageCanvas m_canvas;

    bool m_stroke_active =
        false;

    Option<AtlasHit> m_stroke_origin =
        None();
};

}  // namespace cave