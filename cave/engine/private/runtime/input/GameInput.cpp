#include "GameInput.h"

namespace cave {

using namespace ::cave::literals;

UIInput GameInput::buildUIInput() {
    constexpr int player_id = 0;

    UIInput input{};

    // Mapped UI actions
    input.submit_down = m_state.IsPressed(player_id, "ui_accept"_sid);
    input.submit_pressed = m_state.IsJustPressed(player_id, "ui_accept"_sid);
    input.submit_released = m_state.IsJustReleased(player_id, "ui_accept"_sid);

    input.left_pressed = m_state.IsJustPressed(player_id, "ui_left"_sid);
    input.right_pressed = m_state.IsJustPressed(player_id, "ui_right"_sid);
    input.up_pressed = m_state.IsJustPressed(player_id, "ui_up"_sid);
    input.down_pressed = m_state.IsJustPressed(player_id, "ui_down"_sid);

    return input;
}

}  // namespace cave
