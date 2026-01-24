#pragma once
#include "engine/core/base/singleton.h"
#include "engine/input/action_state.h"
#include "engine/input/input_action_map.h"
#include "engine/input/input_device_interface.h"
#include "engine/input/input_mapper.h"
#include "engine/input/input_router.h"
#include "engine/input/key_state.h"
#include "engine/runtime/module.h"

namespace cave {

struct PointerState {
    bool has_pos = false;
    float x = 0.0f, y = 0.0f;
    float dx = 0.0f, dy = 0.0f;
};

class InputManager : public Module,
                     public Singleton<InputManager> {
public:
    InputManager();

    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    void AddDevice(std::unique_ptr<IInputDevice> p_device);

    void Update();

    const KeyState& GetKeyState() const { return m_key_state; }
    RawInputRouter& RawRouter() { return m_raw_router; }
    InputRouter& Router() { return m_input_router; }
    InputActionMap& ActionMap() { return m_input_action_map; }

    // Convenience overloads for single-player default
    bool IsActionPressed(int p_player, const StringId& p_action) const {
        return m_action_state.IsPressed(p_player, p_action);
    }

    bool IsActionJustPressed(int p_player, const StringId& p_action) const {
        return m_action_state.IsJustPressed(p_player, p_action);
    }

    bool IsActionJustReleased(int p_player, const StringId& p_action) const {
        return m_action_state.IsJustReleased(p_player, p_action);
    }

    float GetActionStrength(int p_player, const StringId& p_action) const {
        return m_action_state.GetStrength(p_player, p_action);
    }

    bool IsActionPressed(const StringId& p_action) const {
        return IsActionPressed(0, p_action);
    }

    bool IsActionJustPressed(const StringId& p_action) const {
        return IsActionJustPressed(0, p_action); 
    }

    bool IsActionJustReleased(const StringId& p_action) const {
        return IsActionJustReleased(0, p_action); 
    }

    float GetActionStrength(const StringId& p_action) const { 
        return GetActionStrength(0, p_action);
    }

    auto GetVector(int p_player,
                   const StringId& p_neg_x,
                   const StringId& p_pos_x,
                   const StringId& p_neg_y,
                   const StringId& p_pos_y) const {
        return m_action_state.GetVector(p_player, p_neg_x, p_pos_x, p_neg_y, p_pos_y);
    }

private:
    void UpdatePointers(std::vector<InputEvent>& p_events);
    void UpdateActions(const DeviceRouting& p_routing);

    std::vector<std::unique_ptr<IInputDevice>> m_devices{};

    std::vector<InputEvent> m_input_events;
    std::vector<ActionEvent> m_action_events;

    std::unordered_map<uint32_t, PointerState> m_pointers;

    KeyState m_key_state;
    ActionState m_action_state;

    RawInputRouter m_raw_router;
    InputRouter m_input_router;

    InputActionMap m_input_action_map;
    InputMapper m_mapper;
};

};  // namespace cave
