#pragma once
#include "cave/runtime/framework/IInputService.h"
#include "cave/runtime/input/KeyState.h"

#include "engine/private/runtime/input/ActionState.h"
#include "engine/private/runtime/input/AxisState.h"
#include "engine/private/runtime/input/InputActionMap.h"
#include "engine/private/runtime/input/InputMapper.h"
#include "engine/private/runtime/input/InputRouter.h"

namespace cave {

struct PointerState {
    bool has_pos = false;
    float x = 0.0f, y = 0.0f;
    float dx = 0.0f, dy = 0.0f;
};

class InputService : public IInputService {
public:
    InputService();

    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    void AddDevice(std::unique_ptr<IInputDevice> p_device) override;

    void Tick(const FrameTime& p_time) override;

    const KeyState& GetKeyState() const override {
        return m_key_state;
    }

    void Register(IInputConsumer* p_consumer) override {
        m_router.Register(p_consumer);
    }

    void Unregister(IInputConsumer* p_consumer) override {
        m_router.Unregister(p_consumer);
    }

    InputActionMap& ActionMap() { return m_input_action_map; }

    bool IsActionPressed(int p_player, const StringId& p_action) const override {
        return m_action_state.IsPressed(p_player, p_action);
    }

    bool IsActionJustPressed(int p_player, const StringId& p_action) const override {
        return m_action_state.IsJustPressed(p_player, p_action);
    }

    bool IsActionJustReleased(int p_player, const StringId& p_action) const override {
        return m_action_state.IsJustReleased(p_player, p_action);
    }

    float GetActionStrength(int p_player, const StringId& p_action) const override {
        return m_action_state.GetStrength(p_player, p_action);
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
    AxisState m_axis_state;
    ActionState m_action_state;

    InputRouter m_router;

    InputActionMap m_input_action_map;
    InputMapper m_mapper;
};

};  // namespace cave
