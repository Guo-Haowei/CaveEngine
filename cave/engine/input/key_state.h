#pragma once

#include "engine/input/input_code.h"
#include "engine/input/input_types.h"

namespace cave {

class InputEvent;

class KeyState {
public:
    // Call once per frame before feeding events
    void BeginFrame();

    // Feed raw events (ButtonDown / ButtonUp)
    void UpdateFromEvents(const InputEvent* events, size_t count);

    // --- Queries ---
    bool Down(InputDeviceId dev, Key k) const;
    bool PressedThisFrame(InputDeviceId dev, Key k) const;
    bool ReleasedThisFrame(InputDeviceId dev, Key k) const;

    // Modifier helpers
    bool CtrlDown(InputDeviceId dev) const;
    bool ShiftDown(InputDeviceId dev) const;
    bool AltDown(InputDeviceId dev) const;

    // Any-device helpers (useful for global shortcuts)
    bool AnyCtrlDown() const;
    bool AnyShiftDown() const;
    bool AnyAltDown() const;

    // Debug / introspection
    void ClearDevice(InputDeviceId dev);

private:
    static size_t Index(Key k);

    struct PerDeviceState {
        std::array<uint8_t, kMaxKeys> down{};
        std::array<uint8_t, kMaxKeys> pressed{};
        std::array<uint8_t, kMaxKeys> released{};
    };

    std::unordered_map<uint32_t, PerDeviceState> m_states;
};

}  // namespace cave
