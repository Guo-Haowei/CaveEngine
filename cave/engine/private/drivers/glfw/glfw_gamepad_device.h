#pragma once
#include "engine/private/input/input_device_interface.h"

namespace cave {

class AxisState;

class GlfwGamepadDevice final : public IInputDevice {
public:
    GlfwGamepadDevice(InputDeviceId p_id, int p_joystick_id)
        : m_id(p_id), m_joy(p_joystick_id) {}

    InputDeviceId Id() const final { return m_id; }
    InputDeviceType Type() const final { return InputDeviceType::Gamepad; }

    void Poll(std::vector<InputEvent>& p_out_events) final;

private:
    InputDeviceId m_id;
    int m_joy = 0;

    std::array<uint8_t, 16> m_prev_buttons{};  // GLFW_GAMEPAD_BUTTON_LAST
};

}  // namespace cave