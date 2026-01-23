#pragma once
#include "engine/core/base/singleton.h"
#include "engine/input/input_device_interface.h"
#include "engine/input/input_router.h"
#include "engine/input/key_state.h"
#include "engine/runtime/module.h"

namespace cave {

class InputManager : public Singleton<InputManager>,
                     public Module {
public:
    InputManager()
        : Module("InputManager") {}

    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    void AddDevice(std::unique_ptr<IInputDevice> p_device);

    // Call once per frame.
    void Update();

    InputRouter& Router() { return m_router; }

private:
    std::vector<std::unique_ptr<IInputDevice>> m_devices{};

    std::vector<InputEvent> m_events;
    std::vector<ActionEvent> m_actions;

    // @TODO: input mapper
    // @TODO: raw router
    KeyState m_key_state;
    InputRouter m_router;
};

};  // namespace cave
