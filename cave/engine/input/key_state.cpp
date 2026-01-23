#include "key_state.h"

namespace cave {

size_t KeyState::Index(Key p_key) {
    const size_t idx = static_cast<size_t>(static_cast<uint16_t>(p_key));
    return (idx < kMaxKeys) ? idx : 0;
}

void KeyState::BeginFrame() {
    // Clear pressed/released for all devices
    for (auto& [_, st] : m_states) {
        st.pressed.reset();
        st.released.reset();
    }
}

void KeyState::UpdateFromEvents(const InputEvent* p_events, size_t p_count) {
    for (size_t i = 0; i < p_count; ++i) {
        const InputEvent& e = p_events[i];
        if (e.consumed) {
            continue;
        }

        if (e.type != InputEventType::ButtonDown &&
            e.type != InputEventType::ButtonUp) {
            continue;
        }

        const Key key = static_cast<Key>(e.code);
        const size_t idx = Index(key);

        auto& st = m_states[e.device.value];  // auto-creates if missing

        if (e.type == InputEventType::ButtonDown) {
            if (!st.down[idx]) {
                st.down[idx] = 1;
                st.pressed[idx] = 1;
            }
        } else {  // e.type == InputEventType::ButtonUp
            if (st.down[idx]) {
                st.down[idx] = 0;
                st.released[idx] = 1;
            }
        }
    }
}

// --- Queries ---

bool KeyState::Down(InputDeviceId p_device, Key p_key) const {
    if (auto it = m_states.find(p_device.value); it != m_states.end()) {
        return it->second.down.test(Index(p_key));
    }

    return false;
}

bool KeyState::PressedThisFrame(InputDeviceId p_device, Key p_key) const {
    if (auto it = m_states.find(p_device.value); it != m_states.end()) {
        return it->second.pressed.test(Index(p_key)) != 0;
    }

    return false;
}

bool KeyState::ReleasedThisFrame(InputDeviceId p_device, Key p_key) const {
    if (auto it = m_states.find(p_device.value); it != m_states.end()) {
        return it->second.released.test(Index(p_key)) != 0;
    }

    return false;
}

// --- Modifiers ---

bool KeyState::CtrlDown(InputDeviceId p_device) const {
    return Down(p_device, Key::LeftCtrl) || Down(p_device, Key::RightCtrl);
}

bool KeyState::ShiftDown(InputDeviceId p_device) const {
    return Down(p_device, Key::LeftShift) || Down(p_device, Key::RightShift);
}

bool KeyState::AltDown(InputDeviceId p_device) const {
    return Down(p_device, Key::LeftAlt) || Down(p_device, Key::RightAlt);
}

bool KeyState::AnyCtrlDown() const {
    for (const auto& [_, st] : m_states) {
        if (st.down[Index(Key::LeftCtrl)] ||
            st.down[Index(Key::RightCtrl)]) {
            return true;
        }
    }
    return false;
}

bool KeyState::AnyShiftDown() const {
    for (const auto& [_, st] : m_states) {
        if (st.down[Index(Key::LeftShift)] ||
            st.down[Index(Key::RightShift)]) {
            return true;
        }
    }
    return false;
}

bool KeyState::AnyAltDown() const {
    for (const auto& [_, st] : m_states) {
        if (st.down[Index(Key::LeftAlt)] ||
            st.down[Index(Key::RightAlt)]) {
            return true;
        }
    }
    return false;
}

void KeyState::ClearDevice(InputDeviceId p_device) {
    m_states.erase(p_device.value);
}

}  // namespace cave
