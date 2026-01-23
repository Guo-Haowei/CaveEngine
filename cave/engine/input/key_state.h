#pragma once

#include "engine/input/key_code.h"
#include "engine/input/input_types.h"

namespace cave {

class KeyState {
public:
    // Call once per frame before feeding events
    void BeginFrame();

    // Feed raw events (ButtonDown / ButtonUp)
    void UpdateFromEvents(const InputEvent* p_events, size_t p_count);

    // --- Queries ---
    bool Down(InputDeviceId p_device, Key p_key) const;
    bool PressedThisFrame(InputDeviceId p_device, Key p_key) const;
    bool ReleasedThisFrame(InputDeviceId p_device, Key p_key) const;

    // Modifier helpers
    bool CtrlDown(InputDeviceId p_device) const;
    bool ShiftDown(InputDeviceId p_device) const;
    bool AltDown(InputDeviceId p_device) const;

    // Any-device helpers (useful for global shortcuts)
    bool AnyCtrlDown() const;
    bool AnyShiftDown() const;
    bool AnyAltDown() const;

    // Debug / introspection
    void ClearDevice(InputDeviceId p_device);

private:
    static size_t Index(Key p_key);

    struct PerDeviceState {
        std::bitset<kMaxKeys> down{};
        std::bitset<kMaxKeys> pressed{};
        std::bitset<kMaxKeys> released{};
    };

    std::unordered_map<uint32_t, PerDeviceState> m_states;
};

}  // namespace cave
