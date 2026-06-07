// =============================================================================
// File: cave/runtime/input/KeyCode.h
// =============================================================================
#pragma once
#include <bitset>
#include <cstdint>
#include <utility>

namespace cave {

// clang-format off
enum class Key : uint16_t {
    None = 0,

    // Keyboard keys
    Space,
    Apostrophe,
    Comma,
    Minus,
    Period,
    Slash,
    _0, _1, _2, _3, _4, _5, _6, _7, _8, _9,
    Semicolon,
    Equal,
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    LeftBracket,
    Backslash,
    RightBracket,
    GraveAccent,
    World1,
    World2,
    Escape,
    Enter,
    Tab,
    Backspace,
    Insert,
    Delete,
    Right,
    Left,
    Down,
    Up,
    PageUp,
    PageDown,
    Home,
    End,
    CapsLock,
    ScrollLock,
    NumLock,
    PrintScreen,
    Pause,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25,
    Keypad0, Keypad1, Keypad2, Keypad3, Keypad4, Keypad5, Keypad6, Keypad7, Keypad8, Keypad9,
    KeypadDecimal,
    KeypadDivide,
    KeypadMultiply,
    KeypadSubtract,
    KeypadAdd,
    KeypadEnter,
    KeypadEqual,
    LeftShift,
    LeftCtrl,
    LeftAlt,
    LeftSuper,
    RightShift,
    RightCtrl,
    RightAlt,
    RightSuper,
    Menu,

    // Mouse buttons
    LMB, // Left Mouse Button
    RMB, // Right Mouse Button
    MMB, // Middle Mouse Button

    // Joystick

    PadA, PadB, PadX, PadY,
    PadLB, PadRB, // Bumper
    PadBack,
    PadStart,
    PadGuid,
    PadLS, PadRS, // Thumb
    PadUp, PadDown, PadLeft, PadRight,

    // Counters
    _Max,

    _FirstKey = Space,
    _LastKey = Menu,
    _FirstMouseButton = LMB,
    _LastMouseButton = MMB,
};
// clang-format on

enum class AxisCode : uint16_t {
    None = 0,
    LX,
    LY,
    RX,
    RY,
    LT,
    RT,

    // Counters
    _Max,
};

constexpr uint16_t kKeyCount = std::to_underlying(Key::_Max);
constexpr uint16_t kAxisCount = std::to_underlying(AxisCode::_Max);

constexpr inline bool IsMouseButton(Key key) {
    return std::to_underlying(key) >= std::to_underlying(Key::_FirstMouseButton) &&
           std::to_underlying(key) <= std::to_underlying(Key::_LastMouseButton);
}

constexpr inline bool IsKey(Key key) {
    return std::to_underlying(key) >= std::to_underlying(Key::_FirstKey) &&
           std::to_underlying(key) <= std::to_underlying(Key::_LastKey);
}

}  // namespace cave
