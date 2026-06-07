#include "UIRuntime.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/view/ViewManager.h"

namespace cave {

using math::Vector2f;

constexpr Color kButtonNormal = Color::Hex(static_cast<ColorCode>(0x303030));
constexpr Color kButtonHover = Color::Hex(static_cast<ColorCode>(0x505050));
constexpr Color kButtonActive = Color::Hex(static_cast<ColorCode>(0x707070));

auto UIRuntime::InitializeImpl() -> Result<void> {
    m_view_manager = m_app->GetViewManager();
    return Result<void>();
}

void UIRuntime::FinalizeImpl() {
}

void UIRuntime::BeginFrame(const UIInput& p_input) {
    m_input = p_input;
    m_draw_data.Clear();
    m_hot = 0;
}

void UIRuntime::EndFrame() {
    if (!m_input.submit_down) {
        m_active = 0;
    }

    DEV_ASSERT(m_stack == 0);
}

void UIRuntime::BeginView(ViewId p_view_id) {
    ++m_stack;
    m_current_view = p_view_id;
}

void UIRuntime::EndView() {
    DEV_ASSERT(m_stack > 0);
    --m_stack;
}

bool UIRuntime::Button(UIId p_id, UIRect p_rect) {
    DEV_ASSERT(m_stack > 0);

    const ViewRecord* view = m_view_manager->Resolve(m_current_view);
    DEV_ASSERT(view);

    const Vector2f point_fb = view->ScreenToFrameBufferPixel(m_input.cursor_os);

    const bool hovered = p_rect.Contains(point_fb.x, point_fb.y);
    if (hovered) {
        m_hot = p_id;
    }

    if (hovered && m_input.submit_pressed) {
        m_active = p_id;
    }

    bool clicked = false;

    if (m_active == p_id && m_input.submit_released) {
        clicked = hovered;
    }

    Color color = kButtonNormal;
    if (m_active == p_id) {
        color = kButtonActive;
    } else if (m_hot == p_id) {
        color = kButtonHover;
    }

    m_draw_data.draw_lists[m_current_view].AddRect(p_rect, color);

    return clicked;
}

}  // namespace cave
