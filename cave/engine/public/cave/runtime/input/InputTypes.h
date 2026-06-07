// =============================================================================
// File: cave/runtime/input/InputTypes.h
// =============================================================================
#pragma once
#include "cave/core/string/StringId.h"

namespace cave {

enum class InputDeviceType : uint8_t {
    None = 0,
    KeyboardMouse,
    Gamepad,
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

enum class ActionEventType : uint8_t {
    Pressed,
    Released,
    Axis1D,
    Axis2D
};

struct InputDeviceId {
    uint32_t value{ 0 };

    constexpr bool operator==(const InputDeviceId& rhs) const { return value == rhs.value; }
    constexpr bool operator!=(const InputDeviceId& rhs) const { return value != rhs.value; }

    inline static constexpr uint8_t kMax = 8;

    static InputDeviceId nextId() {
        static std::atomic<uint32_t> s_id{ 0 };
        return InputDeviceId{ s_id++ };
    }
};

struct InputEvent {
    const InputEventType type{};
    const InputDeviceId dev_id{};
    mutable bool consumed{ false };

    uint32_t code{ 0 };  // Key / Button / Axis index / char / etc.
    float x = 0.0f, y = 0.0f;
    float dx = 0.0f, dy = 0.0f;

    static InputEvent mouseMove(InputDeviceId dev_id,
                                float x,
                                float y) {
        InputEvent e{ InputEventType::MouseMove, dev_id };
        e.x = x;
        e.y = y;
        return e;
    }

    static InputEvent mouseWheel(InputDeviceId dev_id,
                                 float dx,
                                 float dy) {
        InputEvent e{ InputEventType::MouseWheel, dev_id };
        e.x = e.dx = dx;
        e.y = e.dy = dy;
        return e;
    }

    static InputEvent textInput(InputDeviceId dev_id,
                                uint32_t code) {
        InputEvent e{ InputEventType::TextInput, dev_id };
        e.code = code;
        return e;
    }
};

struct ActionEvent {
    StringId action{};
    ActionEventType type{};
    int player = 0;

    float x = 0.0f;
    float y = 0.0f;
};

}  // namespace cave
