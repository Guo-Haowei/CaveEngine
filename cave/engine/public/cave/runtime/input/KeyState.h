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
    void beginFrame();

    // Feed raw events (ButtonDown / ButtonUp)
    void updateFromEvents(const InputEvent* events, size_t count);

    // --- Queries ---
    bool down(InputDeviceId dev_id, Key key) const;
    bool pressedThisFrame(InputDeviceId dev_id, Key key) const;
    bool releasedThisFrame(InputDeviceId dev_id, Key key) const;

    // --- Modifier helpers ---
    bool ctrlDown(InputDeviceId dev_id) const;
    bool shiftDown(InputDeviceId dev_id) const;
    bool altDown(InputDeviceId dev_id) const;

    bool anyCtrlDown() const;
    bool anyShiftDown() const;
    bool anyAltDown() const;

    void clearDevice(InputDeviceId dev_id);
    std::vector<InputDeviceId> activeDevices() const;

private:
    static size_t index(Key key);

    struct PerDeviceState {
        KeyArray down{};
        KeyArray pressed{};
        KeyArray released{};
    };

    std::unordered_map<uint32_t, PerDeviceState> states_;
};

}  // namespace cave
