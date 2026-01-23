#pragma once
#include "engine/input/input_device_interface.h"
#include "engine/input/input_router.h"
#include "engine/input/key_state.h"
#include "engine/runtime/module.h"

namespace cave {

struct PointerState {
    bool has_pos = false;
    float x = 0.0f, y = 0.0f;
    float dx = 0.0f, dy = 0.0f;
};

class InputManager : public Module {
public:
    InputManager()
        : Module("InputManager") {}

    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    void AddDevice(std::unique_ptr<IInputDevice> p_device);

    void Update();

    const KeyState& GetKeyState() const { return m_key_state; }
    RawInputRouter& RawRouter() { return m_raw_router; }
    InputRouter& Router() { return m_router; }

private:
    void UpdatePointers(std::vector<InputEvent>& p_events);

    std::vector<std::unique_ptr<IInputDevice>> m_devices{};

    std::vector<InputEvent> m_events;
    std::vector<ActionEvent> m_actions;

    std::unordered_map<uint32_t, PointerState> m_pointers;

    KeyState m_key_state;
    RawInputRouter m_raw_router;
    InputRouter m_router;
    // @TODO: input mapper
};

};  // namespace cave
