#pragma once
#include "cave/core/math/Vec.h"

namespace cave {

struct ImageCanvasDesc {
    const char* id = "##ImageCanvas";

    ImTextureID texture = 0;

    // Native image size in pixels.
    math::Vec2f image_size_px = math::Vec2f::Zero;

    // Size of the complete widget, including the toolbar.
    // Zero uses the available ImGui content region.
    math::Vec2f widget_size = math::Vec2f::Zero;

    float min_zoom = 0.125f;
    float max_zoom = 16.0f;
    float zoom_step = 1.25f;

    bool show_toolbar = true;
    bool allow_mouse_wheel_zoom = true;
    bool show_checkerboard = false;
};

struct ImageCanvasResult {
    bool visible = false;
    bool hovered = false;

    bool left_clicked = false;
    bool left_double_clicked = false;
    bool right_clicked = false;

    // Mouse position relative to the image's top-left corner,
    // expressed in native image pixels.
    Option<math::Vec2f> mouse_image_px;

    // Image rectangle in ImGui screen coordinates.
    ImVec2 image_min_ss{};
    ImVec2 image_max_ss{};

    ImDrawList* draw_list = nullptr;
};

class ImageCanvas {
public:
    ImageCanvasResult draw(const ImageCanvasDesc& desc);

    float zoom() const {
        return m_zoom;
    }

    void setZoom(float zoom,
                 float min_zoom = 0.125f,
                 float max_zoom = 16.0f);

    void resetZoom() {
        m_zoom = 1.0f;
    }

    void requestFit() {
        m_fit_requested = true;
    }

    math::Vec2f displayedImageSize() const {
        return m_last_image_size_px * m_zoom;
    }

    ImVec2 imageToScreen(math::Vec2f point_px) const;

    Option<math::Vec2f> screenToImage(ImVec2 point_ss) const;

private:
    void drawToolbar(const ImageCanvasDesc& desc,
                     math::Vec2f available_size);

    void applyFit(const ImageCanvasDesc& desc,
                  math::Vec2f available_size);

    void applyMouseWheelZoom(const ImageCanvasDesc& desc,
                             const ImVec2& image_min_ss,
                             const ImVec2& displayed_size);

    static void drawCheckerboard(ImDrawList& draw_list,
                                 ImVec2 min,
                                 ImVec2 max,
                                 float square_size);

private:
    float m_zoom = 1.0f;
    bool m_fit_requested = true;

    math::Vec2f m_last_image_size_px = math::Vec2f::Zero;
    ImVec2 m_last_image_min_ss{};
    ImVec2 m_last_image_max_ss{};
};

}  // namespace cave