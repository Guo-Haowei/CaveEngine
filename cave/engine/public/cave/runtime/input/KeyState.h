// =============================================================================
// File: cave/runtime/input/KeyState.h
// =============================================================================
#pragma once
#include <bitset>
#include <unordered_map>
#include "cave/runtime/input/InputTypes.h"
#include "cave/runtime/input/KeyCode.h"

namespace cave {

class KeyState {
    using KeyArray = std::bitset<kKeyCount>;

public:
    void BeginFrame();

    // Feed raw events (ButtonDown / ButtonUp)
    void UpdateFromEvents(const InputEvent* p_events, size_t p_count);

    // --- Queries ---
    bool Down(InputDeviceId p_device, Key p_key) const;
    bool PressedThisFrame(InputDeviceId p_device, Key p_key) const;
    bool ReleasedThisFrame(InputDeviceId p_device, Key p_key) const;

    // --- Modifier helpers ---
    bool CtrlDown(InputDeviceId p_device) const;
    bool ShiftDown(InputDeviceId p_device) const;
    bool AltDown(InputDeviceId p_device) const;

    bool AnyCtrlDown() const;
    bool AnyShiftDown() const;
    bool AnyAltDown() const;

    void ClearDevice(InputDeviceId p_device);
    std::vector<InputDeviceId> ActiveDevices() const;

private:
    static size_t Index(Key p_key);

    struct PerDeviceState {
        KeyArray down{};
        KeyArray pressed{};
        KeyArray released{};
    };

    std::unordered_map<uint32_t, PerDeviceState> m_states;
};

}  // namespace cave
