#pragma once

namespace cave {

enum class InputDeviceType : uint8_t {
    KeyboardMouse,
    Gamepad,
    JoystickRaw,
};

struct InputDeviceId {
    uint8_t value{ 0 };
    constexpr bool operator==(const InputDeviceId& p_other) const { return value == p_other.value; }
    constexpr bool operator!=(const InputDeviceId& p_other) const { return value != p_other.value; }
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

#if 0

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
    uint32_t code{ 0 };  // Key / Button / Axis index / etc.
    float v0{ 0.0f };
    float v1{ 0.0f };
    uint64_t timestamp_us{ 0 };
    bool consumed{ false };
};
#endif

}  // namespace cave
