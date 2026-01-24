#include "glfw_gamepad_device.h"

#include <GLFW/glfw3.h>
#include "engine/input/key_code.h"

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
            InputEvent e{};
            e.device = m_id;
            e.type = (now == GLFW_PRESS) ? InputEventType::ButtonDown
                                         : InputEventType::ButtonUp;
            e.code = std::to_underlying(k);
            p_out_events.push_back(e);
        }
    }
}

}  // namespace cave
