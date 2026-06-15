#include "cave/runtime/controller/GridSelectController.h"

namespace cave {

GridSelectController::GridSelectController(
    const Vector2i& size,
    Callbacks&& callbacks) noexcept
    : bounds_(size)
    , callbacks_(std::move(callbacks)) {
}

void GridSelectController::clamp() {
    focus_ = math::clamp(focus_,
                         Vector2i::Zero,
                         bounds_ - Vector2i::One);
}

void GridSelectController::focus(int x, int y) {
    if (enabled_) {
        focus_.x = x;
        focus_.y = y;
        clamp();
    }
}

void GridSelectController::moveFocus(int x, int y) {
    if (enabled_) {
        focus_.x += x;
        focus_.y += y;
        clamp();
    }
}

void GridSelectController::cancel() {
    state_ = State::Idle;
    selected_ = None();
    if (callbacks_.on_cancel) {
        callbacks_.on_cancel();
    }
}

void GridSelectController::selectTile(int tx, int ty) {
    if (!enabled_) return;
    if (tx >= bounds_.x || ty >= bounds_.y) return;

    switch (state_) {
        case GridSelectController::State::Idle: {
            stateIdle(tx, ty);
        } break;
        case GridSelectController::State::Armed: {
            stateArmed(tx, ty);
        } break;
    }
}

void GridSelectController::stateIdle(int tx, int ty) {
    if (callbacks_.can_select && !callbacks_.can_select(tx, ty)) {
        if (callbacks_.on_invalid) {
            callbacks_.on_invalid(tx, ty, 0, 0);
        }
        return;
    }

    selected_ = Some(Vector2i(tx, ty));
    state_ = State::Armed;

    if (callbacks_.on_select) {
        callbacks_.on_select(tx, ty);
    }
}

void GridSelectController::stateArmed(int dest_x, int dest_y) {
    DEV_ASSERT(selected_.is_some());
    const int src_x = selected_.unwrap_unchecked().x;
    const int src_y = selected_.unwrap_unchecked().y;

    if (callbacks_.can_drop && !callbacks_.can_drop(src_x, src_y, dest_x, dest_y)) {
        if (callbacks_.on_invalid) {
            callbacks_.on_invalid(src_x, src_y, dest_x, dest_y);
        }
        return;
    }

    if (callbacks_.on_drop) {
        callbacks_.on_drop(src_x, src_y, dest_x, dest_y);
    }
    cancel();
}

}  // namespace cave