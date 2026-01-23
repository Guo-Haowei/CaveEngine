#pragma once

namespace cave {

enum class InputDeviceType : uint8_t {
    KeyboardMouse,
    Gamepad,
    JoystickRaw,
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

using ActionId = uint32_t;

enum class ActionEventType : uint8_t {
    Pressed,
    Released,
    Value,  // axis/analog value updates
};

struct ActionEvent {
    ActionId action = 0;
    ActionEventType type{};
    int player_index = 0;
    float v0 = 0.0f;
    float v1 = 0.0f;
    uint64_t timestamp_us = 0;
};

enum class InputEventType : uint8_t {
    ButtonDown,
    ButtonUp,
    Axis,        // 1D
    MouseMove,   // v0=x, v1=y
    MouseWheel,  // v0=wheel_y
    TextInput,   // code = Unicode codepoint
    DeviceAdded,
    DeviceRemoved,
};

struct InputEvent {
    InputEventType type{};
    InputDeviceId device{};
    bool consumed{ false };

    uint32_t code{ 0 };  // Key / Button / Axis index / char / etc.
    float v0{ 0.0f };
    float v1{ 0.0f };
    uint64_t timestamp_us{ 0 };

    static InputEvent MouseMove(InputDeviceId p_id,
                                uint64_t p_timestamp,
                                float p_x,
                                float p_y) {
        InputEvent e{};
        e.type = InputEventType::MouseMove;
        e.device = p_id;
        e.timestamp_us = p_timestamp;

        e.v0 = p_x;
        e.v1 = p_y;
        return e;
    }

    static InputEvent MouseWheel(InputDeviceId p_id,
                                 uint64_t p_timestamp,
                                 float p_offset) {
        InputEvent e{};
        e.type = InputEventType::MouseWheel;
        e.device = p_id;
        e.timestamp_us = p_timestamp;

        e.v0 = p_offset;
        return e;
    }

    static InputEvent TextInput(InputDeviceId p_id,
                                uint64_t p_timestamp,
                                uint32_t p_code) {
        InputEvent e{};
        e.type = InputEventType::TextInput;
        e.device = p_id;
        e.timestamp_us = p_timestamp;

        e.code = p_code;
        return e;
    }
};

}  // namespace cave
