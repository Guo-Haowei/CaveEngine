#include "GlfwGamepadDevice.h"

#include <GLFW/glfw3.h>

#include "engine/private/runtime/input/AxisState.h"

namespace cave {

static Key toKeyFromGlfwButton(int button) {
    switch (button) {
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

static void EmitAxis(std::vector<InputEvent>& out,
                     InputDeviceId dev_id,
                     AxisCode axis,
                     float value) {
    InputEvent e{ InputEventType::Axis, dev_id };
    e.code = std::to_underlying(axis);
    e.x = value;
    out.push_back(e);
}

void GlfwGamepadDevice::poll(std::vector<InputEvent>& out_events) {
    if (!glfwJoystickPresent(joy_id_)) {
        return;
    }

    // Require gamepad mapping (best for PS5 / Xbox style pads)
    GLFWgamepadstate s{};
    if (!glfwGetGamepadState(joy_id_, &s)) {
        return;
    }

    const int last = GLFW_GAMEPAD_BUTTON_LAST;
    for (int b = 0; b <= last; ++b) {
        const uint8_t now = (uint8_t)s.buttons[b];
        const uint8_t prev = prev_buttons_[(size_t)b];

        if (now == prev) {
            continue;
        }

        prev_buttons_[b] = now;

        if (const Key k = toKeyFromGlfwButton(b); k != Key::None) {
            InputEvent e((now == GLFW_PRESS) ? InputEventType::ButtonDown
                                             : InputEventType::ButtonUp,
                         id_);
            e.code = std::to_underlying(k);
            out_events.push_back(e);
        }
    }

    EmitAxis(out_events, id_, AxisCode::LX, s.axes[GLFW_GAMEPAD_AXIS_LEFT_X]);
    EmitAxis(out_events, id_, AxisCode::LY, s.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
    EmitAxis(out_events, id_, AxisCode::RX, s.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]);
    EmitAxis(out_events, id_, AxisCode::RY, s.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);
    EmitAxis(out_events, id_, AxisCode::LT, s.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]);
    EmitAxis(out_events, id_, AxisCode::RT, s.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]);
}

}  // namespace cave
