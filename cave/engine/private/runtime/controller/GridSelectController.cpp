#include "cave/runtime/controller/GridSelectController.h"

namespace cave {

GridSelectController::GridSelectController(
    const Vector2i& p_size,
    Callbacks&& p_cbs) noexcept
    : m_bounds(p_size)
    , m_cbs(std::move(p_cbs)) {
}

void GridSelectController::SetFocus(const Vector2i& p_focus) {
    m_focus = math::clamp(p_focus,
                          Vector2i::Zero,
                          m_bounds - Vector2i::One);
}

void GridSelectController::MoveFocus(const Vector2i& p_delta) {
    if (m_enabled) {
        SetFocus(m_focus + p_delta);
    }
}

void GridSelectController::Cancel() {
    m_state = State::Idle;
    m_selected = None();
    if (m_cbs.on_cancel) {
        m_cbs.on_cancel();
    }
}

void GridSelectController::SelectTile(int tx, int ty) {
    if (!m_enabled) return;
    if (tx >= m_bounds.x || ty >= m_bounds.y) return;

    switch (m_state) {
        case GridSelectController::State::Idle: {
            StateIdle(tx, ty);
        } break;
        case GridSelectController::State::Armed: {
            StateArmed(tx, ty);
        } break;
    }
}

void GridSelectController::StateIdle(int tx, int ty) {
    if (m_cbs.can_select && !m_cbs.can_select(tx, ty)) {
        if (m_cbs.on_invalid) {
            m_cbs.on_invalid(tx, ty, 0, 0);
        }
        return;
    }

    m_selected = Some(Vector2i(tx, ty));
    m_state = State::Armed;

    if (m_cbs.on_select) {
        m_cbs.on_select(tx, ty);
    }
}

void GridSelectController::StateArmed(int dest_x, int dest_y) {
    DEV_ASSERT(m_selected.is_some());
    const int src_x = m_selected.unwrap_unchecked().x;
    const int src_y = m_selected.unwrap_unchecked().y;

    if (m_cbs.can_drop && !m_cbs.can_drop(src_x, src_y, dest_x, dest_y)) {
        if (m_cbs.on_invalid) {
            m_cbs.on_invalid(src_x, src_y, dest_x, dest_y);
        }
        return;
    }

    if (m_cbs.on_drop) {
        m_cbs.on_drop(src_x, src_y, dest_x, dest_y);
    }
    Cancel();
}

}  // namespace cave