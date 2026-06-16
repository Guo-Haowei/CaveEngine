#include "UIRuntime.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/view/ViewManager.h"

namespace cave {

using math::Vec2f;

constexpr Color kButtonNormal = Color::Hex(static_cast<ColorCode>(0x303030));
constexpr Color kButtonHover = Color::Hex(static_cast<ColorCode>(0x505050));
constexpr Color kButtonActive = Color::Hex(static_cast<ColorCode>(0x707070));

void UIRuntime::beginFrame(const UIInput& input) {
    ui_input_ = input;
    draw_data_.clear();
    hot_ = 0;
}

void UIRuntime::endFrame() {
    if (!ui_input_.submit_down) {
        active_ = 0;
    }

    DEV_ASSERT(stack_ == 0);
}

void UIRuntime::beginView(ViewId view_id) {
    ++stack_;
    current_view_ = view_id;
}

void UIRuntime::endView() {
    DEV_ASSERT(stack_ > 0);
    --stack_;
}

bool UIRuntime::button(UIId uiid, UIRect rect) {
    DEV_ASSERT(stack_ > 0);

    const ViewRecord* view = view_manager_.resolve(current_view_);
    DEV_ASSERT(view);

    const Vec2f point_fb = view->screenToFrameBufferPixel(ui_input_.cursor_os);

    const bool hovered = rect.Contains(point_fb.x, point_fb.y);
    if (hovered) {
        hot_ = uiid;
    }

    if (hovered && ui_input_.submit_pressed) {
        active_ = uiid;
    }

    bool clicked = false;

    if (active_ == uiid && ui_input_.submit_released) {
        clicked = hovered;
    }

    Color color = kButtonNormal;
    if (active_ == uiid) {
        color = kButtonActive;
    } else if (hot_ == uiid) {
        color = kButtonHover;
    }

    draw_data_.draw_lists[current_view_].addRect(rect, color);

    return clicked;
}

}  // namespace cave
