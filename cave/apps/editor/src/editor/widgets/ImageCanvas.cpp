#include "ImageCanvas.h"

namespace cave {

using namespace ::cave::math;

namespace {

ImVec2 ToImVec2(Vec2f value) {
    return ImVec2{ value.x, value.y };
}

Vec2f ToVec2f(ImVec2 value) {
    return Vec2f{ value.x, value.y };
}

float ClampZoom(float zoom, float min_zoom, float max_zoom) {
    return std::clamp(zoom, min_zoom, max_zoom);
}

}  // namespace

void ImageCanvas::setZoom(float zoom,
                          float min_zoom,
                          float max_zoom) {
    m_zoom = ClampZoom(zoom, min_zoom, max_zoom);
    m_fit_requested = false;
}

ImVec2 ImageCanvas::imageToScreen(Vec2f point_px) const {
    return ImVec2{
        m_last_image_min_ss.x + point_px.x * m_zoom,
        m_last_image_min_ss.y + point_px.y * m_zoom,
    };
}

Option<Vec2f> ImageCanvas::screenToImage(ImVec2 point_ss) const {
    if (m_zoom <= 0.0f) {
        return None();
    }

    if (point_ss.x < m_last_image_min_ss.x ||
        point_ss.y < m_last_image_min_ss.y ||
        point_ss.x >= m_last_image_max_ss.x ||
        point_ss.y >= m_last_image_max_ss.y) {
        return None();
    }

    return Some(Vec2f{
        (point_ss.x - m_last_image_min_ss.x) / m_zoom,
        (point_ss.y - m_last_image_min_ss.y) / m_zoom,
    });
}

void ImageCanvas::drawToolbar(const ImageCanvasDesc& desc,
                              Vec2f available_size) {
    const float button_width = ImGui::GetFrameHeight();

    if (ImGui::Button("-", ImVec2{ button_width, 0.0f })) {
        setZoom(
            m_zoom / desc.zoom_step,
            desc.min_zoom,
            desc.max_zoom);
    }

    ImGui::SameLine();

    char zoom_text[32];
    std::snprintf(
        zoom_text,
        sizeof(zoom_text),
        "%.0f%%",
        m_zoom * 100.0f);

    const float zoom_text_width =
        std::max(64.0f, ImGui::CalcTextSize(zoom_text).x + 20.0f);

    ImGui::SetNextItemWidth(zoom_text_width);

    if (ImGui::Button(
            zoom_text,
            ImVec2{ zoom_text_width, 0.0f })) {
        resetZoom();
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Reset zoom to 100%%");
    }

    ImGui::SameLine();

    if (ImGui::Button("+", ImVec2{ button_width, 0.0f })) {
        setZoom(
            m_zoom * desc.zoom_step,
            desc.min_zoom,
            desc.max_zoom);
    }

    ImGui::SameLine();

    if (ImGui::Button("1:1")) {
        resetZoom();
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Actual image size");
    }

    ImGui::SameLine();

    if (ImGui::Button("Fit")) {
        m_fit_requested = true;
        applyFit(desc, available_size);
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Fit image to available space");
    }
}

void ImageCanvas::applyFit(const ImageCanvasDesc& desc,
                           Vec2f available_size) {
    if (desc.image_size_px.x <= 0.0f ||
        desc.image_size_px.y <= 0.0f ||
        available_size.x <= 0.0f ||
        available_size.y <= 0.0f) {
        return;
    }

    const float zoom_x =
        available_size.x / desc.image_size_px.x;

    const float zoom_y =
        available_size.y / desc.image_size_px.y;

    m_zoom = ClampZoom(
        std::min(zoom_x, zoom_y),
        desc.min_zoom,
        desc.max_zoom);

    m_fit_requested = false;
}

void ImageCanvas::applyMouseWheelZoom(
    const ImageCanvasDesc& desc,
    const ImVec2& image_min_ss,
    const ImVec2& displayed_size) {

    if (!desc.allow_mouse_wheel_zoom ||
        !ImGui::IsWindowHovered(
            ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
        return;
    }

    const float wheel = ImGui::GetIO().MouseWheel;
    if (wheel == 0.0f) {
        return;
    }

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (!window) {
        return;
    }

    const ImVec2 mouse_ss = ImGui::GetMousePos();

    const float old_zoom = m_zoom;

    const float factor =
        wheel > 0.0f
            ? std::pow(desc.zoom_step, wheel)
            : std::pow(desc.zoom_step, wheel);

    const float new_zoom = ClampZoom(
        old_zoom * factor,
        desc.min_zoom,
        desc.max_zoom);

    if (std::abs(new_zoom - old_zoom) < 0.000001f) {
        return;
    }

    // Preserve the texel under the cursor by adjusting the child scroll.
    const float mouse_image_x =
        (mouse_ss.x - image_min_ss.x) / old_zoom;

    const float mouse_image_y =
        (mouse_ss.y - image_min_ss.y) / old_zoom;

    const float new_display_x =
        mouse_image_x * new_zoom;

    const float new_display_y =
        mouse_image_y * new_zoom;

    const float old_display_x =
        mouse_image_x * old_zoom;

    const float old_display_y =
        mouse_image_y * old_zoom;

    const float scroll_x =
        ImGui::GetScrollX() +
        (new_display_x - old_display_x);

    const float scroll_y =
        ImGui::GetScrollY() +
        (new_display_y - old_display_y);

    m_zoom = new_zoom;
    m_fit_requested = false;

    ImGui::SetScrollX(scroll_x);
    ImGui::SetScrollY(scroll_y);

    (void)displayed_size;
}

void ImageCanvas::drawCheckerboard(ImDrawList& draw_list,
                                   ImVec2 min,
                                   ImVec2 max,
                                   float square_size) {
    const ImU32 light =
        IM_COL32(204, 204, 204, 255);

    const ImU32 dark =
        IM_COL32(136, 136, 136, 255);

    int row = 0;

    for (float y = min.y; y < max.y; y += square_size, ++row) {
        int column = 0;

        for (float x = min.x; x < max.x; x += square_size, ++column) {
            const ImVec2 cell_min{ x, y };

            const ImVec2 cell_max{
                std::min(x + square_size, max.x),
                std::min(y + square_size, max.y),
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

ImageCanvasResult ImageCanvas::draw(
    const ImageCanvasDesc& desc) {

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

        Vec2f fit_area{
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

    const bool child_visible = ImGui::BeginChild(
        "##ImageCanvasScroll",
        ToImVec2(child_size),
        ImGuiChildFlags_Borders,
        child_flags);

    result.visible = child_visible;

    if (!child_visible) {
        ImGui::EndChild();
        ImGui::PopID();
        return result;
    }

    m_last_image_size_px = desc.image_size_px;

    Vec2f content_size =
        ToVec2f(ImGui::GetContentRegionAvail());

    if (m_fit_requested) {
        applyFit(desc, content_size);
    }

    const Vec2f displayed_size{
        std::max(desc.image_size_px.x * m_zoom, 1.0f),
        std::max(desc.image_size_px.y * m_zoom, 1.0f),
    };

    // Center the image when it is smaller than the viewport.
    const Vec2f padding{
        std::max(
            (content_size.x - displayed_size.x) * 0.5f,
            0.0f),

        std::max(
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
            std::max(8.0f * m_zoom, 4.0f));
    }

    if (desc.texture != 0) {
        ImGui::Image(
            desc.texture,
            ToImVec2(displayed_size));
    } else {
        ImGui::InvisibleButton(
            "##MissingImage",
            ToImVec2(displayed_size));
    }

    result.hovered = ImGui::IsItemHovered();
    result.left_clicked =
        result.hovered &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left);

    result.left_double_clicked =
        result.hovered &&
        ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

    result.right_clicked =
        result.hovered &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Right);

    result.image_min_ss = image_min_ss;
    result.image_max_ss = image_max_ss;
    result.draw_list = draw_list;

    m_last_image_min_ss = image_min_ss;
    m_last_image_max_ss = image_max_ss;

    if (result.hovered) {
        const ImVec2 mouse_ss =
            ImGui::GetMousePos();

        result.mouse_image_px = Some(Vec2f{
            (mouse_ss.x - image_min_ss.x) / m_zoom,
            (mouse_ss.y - image_min_ss.y) / m_zoom,
        });
    }

    applyMouseWheelZoom(
        desc,
        image_min_ss,
        ToImVec2(displayed_size));

    ImGui::EndChild();
    ImGui::PopID();

    return result;
}

}  // namespace cave