#include "GameInput.h"

namespace cave {

using namespace ::cave::literals;

UIInput GameInput::BuildUIInput() {
    UIInput input{};

    const InputDeviceId device_id{ 0 };
    const int player_id = 0;

    // DisplayService& display_service = *m_app->GetDisplayService();

    // const auto it = m_pointers.find(device_id.value);
    // if (it != m_pointers.end()) {
    //     const PointerState pointer = it->second;
    //     const auto [x_win, y_win] = display_service.GetWindowPos();
    //     input.cursor_os = math::Vector2f(pointer.x + x_win, pointer.y + y_win);
    // }

    // Mapped UI actions
    input.submit_pressed = m_state.IsJustPressed(player_id, "ui_accept"_sid);
    input.cancel_pressed = m_state.IsJustPressed(player_id, "ui_back"_sid);

    input.left_pressed = m_state.IsJustPressed(player_id, "ui_left"_sid);
    input.right_pressed = m_state.IsJustPressed(player_id, "ui_right"_sid);
    input.up_pressed = m_state.IsJustPressed(player_id, "ui_up"_sid);
    input.down_pressed = m_state.IsJustPressed(player_id, "ui_down"_sid);

    return input;
}

}  // namespace cave
