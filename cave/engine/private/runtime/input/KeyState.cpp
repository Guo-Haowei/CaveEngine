#include "cave/runtime/input/KeyState.h"

namespace cave {

size_t KeyState::index(Key key) {
    const size_t idx = static_cast<size_t>(static_cast<uint16_t>(key));
    return (idx < kKeyCount) ? idx : 0;
}

void KeyState::beginFrame() {
    // Clear pressed/released for all devices
    for (auto& [_, st] : m_states) {
        st.pressed.reset();
        st.released.reset();
    }
}

void KeyState::updateFromEvents(const InputEvent* events, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        const InputEvent& e = events[i];
        if (e.consumed) {
            continue;
        }

        if (e.type != InputEventType::ButtonDown &&
            e.type != InputEventType::ButtonUp) {
            continue;
        }

        const Key key = static_cast<Key>(e.code);
        const size_t idx = index(key);

        auto& st = m_states[e.dev_id.value];  // auto-creates if missing

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

bool KeyState::down(InputDeviceId dev_id, Key key) const {
    if (auto it = m_states.find(dev_id.value); it != m_states.end()) {
        return it->second.down.test(index(key));
    }

    return false;
}

bool KeyState::pressedThisFrame(InputDeviceId dev_id, Key key) const {
    if (auto it = m_states.find(dev_id.value); it != m_states.end()) {
        return it->second.pressed.test(index(key)) != 0;
    }

    return false;
}

bool KeyState::releasedThisFrame(InputDeviceId dev_id, Key key) const {
    if (auto it = m_states.find(dev_id.value); it != m_states.end()) {
        return it->second.released.test(index(key)) != 0;
    }

    return false;
}

// --- Modifiers ---

bool KeyState::ctrlDown(InputDeviceId dev_id) const {
    return down(dev_id, Key::LeftCtrl) || down(dev_id, Key::RightCtrl);
}

bool KeyState::shiftDown(InputDeviceId dev_id) const {
    return down(dev_id, Key::LeftShift) || down(dev_id, Key::RightShift);
}

bool KeyState::altDown(InputDeviceId dev_id) const {
    return down(dev_id, Key::LeftAlt) || down(dev_id, Key::RightAlt);
}

bool KeyState::anyCtrlDown() const {
    for (const auto& [_, st] : m_states) {
        if (st.down[index(Key::LeftCtrl)] ||
            st.down[index(Key::RightCtrl)]) {
            return true;
        }
    }
    return false;
}

bool KeyState::anyShiftDown() const {
    for (const auto& [_, st] : m_states) {
        if (st.down[index(Key::LeftShift)] ||
            st.down[index(Key::RightShift)]) {
            return true;
        }
    }
    return false;
}

bool KeyState::anyAltDown() const {
    for (const auto& [_, st] : m_states) {
        if (st.down[index(Key::LeftAlt)] ||
            st.down[index(Key::RightAlt)]) {
            return true;
        }
    }
    return false;
}

void KeyState::clearDevice(InputDeviceId dev_id) {
    m_states.erase(dev_id.value);
}

std::vector<InputDeviceId> KeyState::activeDevices() const {
    std::vector<InputDeviceId> devices;
    for (const auto& [key, _] : m_states) {
        devices.push_back(InputDeviceId{ key });
    }
    return devices;
}

}  // namespace cave
