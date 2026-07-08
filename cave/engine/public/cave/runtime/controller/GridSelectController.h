#pragma once
#include <cstdint>
#include <functional>
#include "cave/core/Option.h"
#include "cave/core/math/Vec.h"

namespace cave {

//-- Optional mouse support later (additive)
//-- function update_hover_from_screen(sx, sy)
//--   if not (self.grid.screen_to_tile) then return end
//--   local tx, ty = self.grid.screen_to_tile(sx, sy)
//--   if tx then self.hover = { x = tx, y = ty } else self.hover = nil end
//-- end
//
//-- function click_from_screen(sx, sy)
//--   if not (self.grid.screen_to_tile) then return end
//--   local tx, ty = self.grid.screen_to_tile(sx, sy)
//--   if tx then self:select_tile(tx, ty) end
//-- end

class GridSelectController {
    using Vector2i = math::Vec2i;

    enum class State : uint8_t {
        Idle = 0,
        Armed,
    };

public:
    struct Callbacks {
        std::function<bool(int, int)> can_select{};
        std::function<void(int, int)> on_select{};

        std::function<bool(int, int, int, int)> can_drop{};
        std::function<void(int, int, int, int)> on_drop{};

        std::function<void()> on_cancel{};
        std::function<void(int, int, int, int)> on_invalid{};
    };

    explicit GridSelectController(const Vector2i& size, Callbacks&& callbacks) noexcept;

    void focus(int x, int y);

    void moveFocus(int x, int y);

    bool isArmed() const { return state_ == State::Armed; }

    void confirm() { selectTile(focus_.x, focus_.y); }

    void cancel();

    const Vector2i& focus() const { return focus_; }

private:
    void selectTile(int tx, int ty);
    void clamp();

    void stateIdle(int tx, int ty);
    void stateArmed(int dest_x, int dest_y);

    const Vector2i bounds_;
    Vector2i focus_{ 0 };
    Callbacks callbacks_{};

    State state_{ State::Idle };
    bool enabled_{ true };

    Option<Vector2i> selected_;
};

}  // namespace cave