#pragma once
#include "engine/input/input_device_interface.h"
#include "engine/input/input_router.h"

namespace cave {

class KeyState;

class InputSystem {
public:
    void AddDevice(std::unique_ptr<IInputDevice> p_device);

    // Call once per frame.
    void Tick();

    InputRouter& Router() { return m_router; }

    // Optional: expose for debug
    const std::vector<IInputDevice::Event>& DebugEvents() const { return m_events; }

private:
    std::vector<std::unique_ptr<IInputDevice>> m_devices;

    std::vector<IInputDevice::Event> m_events;
    std::vector<ActionEvent> m_actions;

    // InputMapper m_mapper;
    std::unique_ptr<KeyState> m_keys;
    InputRouter m_router;
};

}  // namespace cave
