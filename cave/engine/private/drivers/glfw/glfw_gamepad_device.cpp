#include "glfw_gamepad_device.h"

#include <GLFW/glfw3.h>

#include "engine/private/input/axis_state.h"

namespace cave {

static Key ToKeyFromGlfwButton(int p_button) {
    switch (p_button) {
        case GLFW_GAMEPAD_BUTTON_A:
            return Key::PadA;
        case GLFW_GAMEPAD_BUTTON_B:
            return Key::PadB;
        case GLFW_GAMEPAD_BUTTON_X:
            return Key::PadX;
        case GLFW_GAMEPAD_BUTTON_Y:
            return Key::PadY;

        case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER:
            return Key::PadLB;
        case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER:
            return Key::PadRB;

        case GLFW_GAMEPAD_BUTTON_BACK:
            return Key::PadBack;
        case GLFW_GAMEPAD_BUTTON_START:
            return Key::PadStart;

        case GLFW_GAMEPAD_BUTTON_DPAD_UP:
            return Key::PadUp;
        case GLFW_GAMEPAD_BUTTON_DPAD_DOWN:
            return Key::PadDown;
        case GLFW_GAMEPAD_BUTTON_DPAD_LEFT:
            return Key::PadLeft;
        case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT:
            return Key::PadRight;

        case GLFW_GAMEPAD_BUTTON_LEFT_THUMB:
            return Key::PadLS;
        case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB:
            return Key::PadRS;

        default:
            return Key::None;
    }
}

static void EmitAxis(std::vector<InputEvent>& p_out,
                     InputDeviceId p_dev_id,
                     AxisCode p_axis,
                     float p_value) {
    InputEvent e(InputEventType::Axis, p_dev_id);
    e.code = std::to_underlying(p_axis);
    e.x = p_value;
    p_out.push_back(e);
}

void GlfwGamepadDevice::Poll(std::vector<InputEvent>& p_out_events) {
    if (!glfwJoystickPresent(m_joy)) {
        return;
    }

    // Require gamepad mapping (best for PS5 / Xbox style pads)
    GLFWgamepadstate s{};
    if (!glfwGetGamepadState(m_joy, &s)) {
        return;
    }

    const int last = GLFW_GAMEPAD_BUTTON_LAST;
    for (int b = 0; b <= last; ++b) {
        const uint8_t now = (uint8_t)s.buttons[b];
        const uint8_t prev = m_prev_buttons[(size_t)b];

        if (now == prev) {
            continue;
        }

        m_prev_buttons[b] = now;

        if (const Key k = ToKeyFromGlfwButton(b); k != Key::None) {
            InputEvent e((now == GLFW_PRESS) ? InputEventType::ButtonDown
                                             : InputEventType::ButtonUp,
                         m_id);
            e.code = std::to_underlying(k);
            p_out_events.push_back(e);
        }
    }

    EmitAxis(p_out_events, m_id, AxisCode::LX, s.axes[GLFW_GAMEPAD_AXIS_LEFT_X]);
    EmitAxis(p_out_events, m_id, AxisCode::LY, s.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
    EmitAxis(p_out_events, m_id, AxisCode::RX, s.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]);
    EmitAxis(p_out_events, m_id, AxisCode::RY, s.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);
    EmitAxis(p_out_events, m_id, AxisCode::LT, s.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]);
    EmitAxis(p_out_events, m_id, AxisCode::RT, s.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]);
}

}  // namespace cave
