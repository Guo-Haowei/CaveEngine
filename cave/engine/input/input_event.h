#pragma once

#include "engine/input/input_types.h"

// @TODO: refactor
#include "input_code.h"
#include "engine/math/geomath.h"

namespace cave {

enum class InputState : uint8_t {
    UNKNOWN = 0,
    PRESSED,
    HOLD,
    RELEASED,
    // @TODO: TOGGLE?
};

class OldInputEvent {
public:
    virtual ~OldInputEvent() = default;

    bool IsAltPressed() const { return m_alt_pressed; }
    bool IsShiftPressed() const { return m_shift_pressed; }
    bool IsCtrlPressed() const { return m_ctrl_pressed; }
    bool IsModiferPressed() const { return m_alt_pressed || m_shift_pressed || m_ctrl_pressed; }

protected:
    bool m_alt_pressed;
    bool m_shift_pressed;
    bool m_ctrl_pressed;

    friend class InputManager;
};

class InputEventKey : public OldInputEvent {
public:
    bool IsPressed() const { return m_state == InputState::PRESSED; }
    bool IsHolding() const { return m_state == InputState::HOLD; }
    bool IsReleased() const { return m_state == InputState::RELEASED; }

    Key GetKey() const { return m_key; }

protected:
    Key m_key;
    InputState m_state;

    friend class InputManager;
};


template<size_t N>
inline bool InputIsDown(const std::bitset<N>& p_array, int p_index) {
    DEV_ASSERT_INDEX(p_index, N);
    return p_array[p_index];
}

template<size_t N>
inline bool InputHasChanged(const std::bitset<N>& p_current, const std::bitset<N>& p_prev, int p_index) {
    DEV_ASSERT_INDEX(p_index, N);
    return p_current[p_index] == true && p_prev[p_index] == false;
}

}  // namespace cave
