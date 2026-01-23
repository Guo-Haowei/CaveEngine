#include "key_state.h"

namespace cave {

size_t KeyState::Index(Key k) {
    const size_t idx = static_cast<size_t>(static_cast<uint16_t>(k));
    return (idx < kMaxKeys) ? idx : 0;
}

void KeyState::BeginFrame() {
    // Clear pressed/released for all devices
    for (auto& [_, st] : m_states) {
        st.pressed.fill(0);
        st.released.fill(0);
    }
}

void KeyState::UpdateFromEvents(const InputEvent* events, size_t count) {
    unused(events);
    unused(count);
    CRASH_NOW();
#if 0
    for (size_t i = 0; i < count; ++i) {
        const InputEvent& e = events[i];
        if (e.consumed) continue;

        if (e.type != InputEventType::ButtonDown &&
            e.type != InputEventType::ButtonUp) {
            continue;
        }

        const Key k = FromCode(e.code);
        const size_t idx = Index(k);

        auto& st = m_states[e.device.value];  // auto-creates if missing

        if (e.type == InputEventType::ButtonDown) {
            if (!st.down[idx]) {
                st.down[idx] = 1;
                st.pressed[idx] = 1;
            }
        } else {  // ButtonUp
            if (st.down[idx]) {
                st.down[idx] = 0;
                st.released[idx] = 1;
            }
        }
    }
#endif
}

// --- Queries ---

bool KeyState::Down(InputDeviceId dev, Key k) const {
    auto it = m_states.find(dev.value);
    if (it == m_states.end()) return false;
    return it->second.down[Index(k)] != 0;
}

bool KeyState::PressedThisFrame(InputDeviceId dev, Key k) const {
    auto it = m_states.find(dev.value);
    if (it == m_states.end()) return false;
    return it->second.pressed[Index(k)] != 0;
}

bool KeyState::ReleasedThisFrame(InputDeviceId dev, Key k) const {
    auto it = m_states.find(dev.value);
    if (it == m_states.end()) return false;
    return it->second.released[Index(k)] != 0;
}

// --- Modifiers ---

bool KeyState::CtrlDown(InputDeviceId dev) const {
    return Down(dev, Key::KEY_LEFT_CONTROL) || Down(dev, Key::KEY_RIGHT_CONTROL);
}

bool KeyState::ShiftDown(InputDeviceId dev) const {
    return Down(dev, Key::KEY_LEFT_SHIFT) || Down(dev, Key::KEY_RIGHT_SHIFT);
}

bool KeyState::AltDown(InputDeviceId dev) const {
    return Down(dev, Key::KEY_LEFT_ALT) || Down(dev, Key::KEY_RIGHT_ALT);
}

bool KeyState::AnyCtrlDown() const {
    for (const auto& [_, st] : m_states) {
#if 0
        if (st.down[Index(Key::LeftCtrl)] ||
            st.down[Index(Key::RightCtrl)]) {
            return true;
        }
#endif
    }
    return false;
}

bool KeyState::AnyShiftDown() const {
    for (const auto& [_, st] : m_states) {
#if 0
        if (st.down[Index(Key::LeftShift)] ||
            st.down[Index(Key::RightShift)]) {
            return true;
        }
#endif
    }
    return false;
}

bool KeyState::AnyAltDown() const {
    for (const auto& [_, st] : m_states) {
#if 0
        if (st.down[Index(Key::LeftAlt)] ||
            st.down[Index(Key::RightAlt)]) {
            return true;
        }
#endif
    }
    return false;
}

void KeyState::ClearDevice(InputDeviceId dev) {
    m_states.erase(dev.value);
}

}  // namespace cave
