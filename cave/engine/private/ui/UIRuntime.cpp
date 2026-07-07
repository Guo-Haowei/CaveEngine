#include "UIRuntime.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/view/ViewManager.h"

namespace cave {

using math::Vec2f;

constexpr Color kButtonNormal = Color::Hex(static_cast<ColorCode>(0x303030));
constexpr Color kButtonHover = Color::Hex(static_cast<ColorCode>(0x505050));
constexpr Color kButtonActive = Color::Hex(static_cast<ColorCode>(0x707070));

void UIRuntime::beginFrame(const UIInput& input) {
    m_ui_input = input;
    m_draw_data.clear();
    m_hot = 0;
}

void UIRuntime::endFrame() {
    if (!m_ui_input.submit_down) {
        m_active = 0;
    }

    DEV_ASSERT(m_stack == 0);
}

void UIRuntime::beginView(ViewId view_id) {
    ++m_stack;
    m_current_view = view_id;
}

void UIRuntime::endView() {
    DEV_ASSERT(m_stack > 0);
    --m_stack;
}

bool UIRuntime::button(UIId uiid, UIRect rect) {
    DEV_ASSERT(m_stack > 0);

    const ViewRecord* view = m_view_manager.resolve(m_current_view);
    DEV_ASSERT(view);

    const Vec2f point_fb = view->screenToFrameBufferPixel(m_ui_input.cursor_os);

    const bool hovered = rect.Contains(point_fb.x, point_fb.y);
    if (hovered) {
        m_hot = uiid;
    }

    if (hovered && m_ui_input.submit_pressed) {
        m_active = uiid;
    }

    bool clicked = false;

    if (m_active == uiid && m_ui_input.submit_released) {
        clicked = hovered;
    }

    Color color = kButtonNormal;
    if (m_active == uiid) {
        color = kButtonActive;
    } else if (m_hot == uiid) {
        color = kButtonHover;
    }

    m_draw_data.draw_lists[m_current_view].addRect(rect, color);

    return clicked;
}

}  // namespace cave
