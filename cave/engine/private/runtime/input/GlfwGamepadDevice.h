#pragma once
#include "cave/runtime/input/IInputDevice.h"

namespace cave {

class AxisState;

class GlfwGamepadDevice final : public IInputDevice {
public:
    GlfwGamepadDevice(InputDeviceId p_id, int p_joystick_id)
        : id_(p_id), joy_id_(p_joystick_id) {}

    InputDeviceType type() const override { return InputDeviceType::Gamepad; }
    InputDeviceId id() const override { return id_; }

    void poll(std::vector<InputEvent>& out_events) override;

private:
    InputDeviceId id_;
    int joy_id_ = 0;

    std::array<uint8_t, 16> prev_buttons_{};  // GLFW_GAMEPAD_BUTTON_LAST
};

}  // namespace cave