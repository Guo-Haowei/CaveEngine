#pragma once
#include <cstdint>
#include <functional>
#include "cave/core/Option.h"
#include "cave/core/math/Vector.h"

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
    using Vector2i = math::Vector2i;

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

    explicit GridSelectController(const Vector2i& p_size, Callbacks&& p_cbs) noexcept;

    void SetFocus(int p_x, int p_y);

    void MoveFocus(int p_x, int p_y);

    bool IsArmed() const { return m_state == State::Armed; }

    void Confirm() { SelectTile(m_focus.x, m_focus.y); }

    void Cancel();

    const Vector2i& GetFocused() const { return m_focus; }

private:
    void SelectTile(int tx, int ty);
    void Clamp();

    void StateIdle(int tx, int ty);
    void StateArmed(int dest_x, int dest_y);

    const Vector2i m_bounds;
    Vector2i m_focus{ 0 };
    Callbacks m_cbs{};

    State m_state{ State::Idle };
    bool m_enabled{ true };

    Option<Vector2i> m_selected;
};

}  // namespace cave