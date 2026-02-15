// =============================================================================
// File: public/cave/runtime/input/InputTypes.h
// =============================================================================
#pragma once
#include "cave/core/string/StringId.h"

namespace cave {

enum class InputDeviceType : uint8_t {
    None = 0,
    KeyboardMouse,
    Gamepad,
};

struct InputDeviceId {
    uint32_t value{ 0 };
    constexpr bool operator==(const InputDeviceId& p_other) const { return value == p_other.value; }
    constexpr bool operator!=(const InputDeviceId& p_other) const { return value != p_other.value; }

    static InputDeviceId NextId() {
        static std::atomic<uint32_t> s_id{ 0 };
        return InputDeviceId{ s_id++ };
    }
};

enum class InputEventType : uint8_t {
    ButtonDown,
    ButtonUp,
    Axis,
    MouseMove,   // v0=x, v1=y
    MouseWheel,  // v0=wheel_y
    TextInput,   // code = Unicode codepoint
    DeviceAdded,
    DeviceRemoved,
};

struct InputEvent {
    const InputEventType type{};
    const InputDeviceId device_id{};
    mutable bool consumed{ false };

    uint32_t code{ 0 };  // Key / Button / Axis index / char / etc.
    float x = 0.0f, y = 0.0f;
    float dx = 0.0f, dy = 0.0f;

    InputEvent(InputEventType p_type, InputDeviceId p_dev_id)
        : type(p_type)
        , device_id(p_dev_id) {
    }

    static InputEvent MouseMove(InputDeviceId p_dev_id,
                                float p_x,
                                float p_y) {
        InputEvent e(InputEventType::MouseMove, p_dev_id);
        e.x = p_x;
        e.y = p_y;
        return e;
    }

    static InputEvent MouseWheel(InputDeviceId p_dev_id,
                                 float p_x_offset,
                                 float p_y_offset) {
        InputEvent e(InputEventType::MouseWheel, p_dev_id);

        e.x = e.dx = p_x_offset;
        e.y = e.dy = p_y_offset;
        return e;
    }

    static InputEvent TextInput(InputDeviceId p_dev_id,
                                uint32_t p_code) {
        InputEvent e(InputEventType::TextInput, p_dev_id);

        e.code = p_code;
        return e;
    }
};

enum class ActionEventType : uint8_t {
    Pressed,
    Released,
    Axis1D,
    Axis2D
};

struct ActionEvent {
    StringId action;
    ActionEventType type;
    int player = 0;

    float x = 0.0f;
    float y = 0.0f;
};

}  // namespace cave
