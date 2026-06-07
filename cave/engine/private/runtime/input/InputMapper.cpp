// =============================================================================
// File: engine/private/runtime/input/InputMapper.cpp
// =============================================================================
#include "InputMapper.h"

#include "AxisState.h"
#include "cave/runtime/input/KeyState.h"

namespace cave {

void InputMapper::Map(const std::vector<InputEvent>& p_events,
                      const KeyState& p_key_state,
                      const AxisState& p_axis_state,
                      const DeviceRouting& p_routing,
                      std::vector<ActionEvent>& p_out_actions) const {
    for (const auto& [name, def] : m_map.GetActions()) {
        switch (def.type) {
            case ActionValueType::Digital: {
                MapDigital(name, def, p_events, p_routing, p_out_actions);
            } break;
            case ActionValueType::Scalar: {
                MapScalar(name, def, p_key_state, p_routing, p_out_actions);
                MapScalar(name, def, p_axis_state, p_routing, p_out_actions);
            } break;
        }
    }
}

void InputMapper::MapDigital(const StringId& p_str_id,
                             const ActionDef& p_def,
                             const std::vector<InputEvent>& p_events,
                             const DeviceRouting& p_routing,
                             std::vector<ActionEvent>& p_out_actions) const {
    for (const auto& e : p_events) {
        if (e.consumed) {
            continue;
        }

        if (e.type != InputEventType::ButtonDown && e.type != InputEventType::ButtonUp) {
            continue;
        }

        const Key k = static_cast<Key>(e.code);

        for (const ActionBinding& binding : p_def.bindings) {
            if (binding.behavior != BindingBehavior::Digital) {
                continue;
            }
            if (binding.source.key != k) {
                continue;
            }

            ActionEvent action{};
            action.action = p_str_id;
            action.type = (e.type == InputEventType::ButtonDown)
                              ? ActionEventType::Pressed
                              : ActionEventType::Released;
            action.player = p_routing.PlayerFor(e.device_id);

            p_out_actions.push_back(action);
            break;
        }
    }
}

void InputMapper::MapScalar(const StringId& p_str_id, const ActionDef& p_def,
                            const KeyState& p_keys,
                            const DeviceRouting& p_routing,
                            std::vector<ActionEvent>& p_out_actions) const {

    for (InputDeviceId dev_id : p_keys.ActiveDevices()) {
        // Typical editor rule: don�t drive movement while Ctrl/Alt are down
        // (Put your own gating elsewhere if you prefer)
        // if (keys.CtrlDown(dev) || keys.AltDown(dev)) continue;

        float value = 0.0f;

        for (const ActionBinding& binding : p_def.bindings) {
            if (binding.behavior != BindingBehavior::Scalar) {
                continue;
            }

            if (binding.source.type == BindingSourceType::Key) {
                if (p_keys.Down(dev_id, binding.source.key)) {
                    value += binding.scale;
                }
            }
        }

        if (value != 0.0f) {
            ActionEvent action{};
            action.action = p_str_id;
            action.type = ActionEventType::Axis1D;
            action.player = p_routing.PlayerFor(dev_id);
            action.x = value;
            p_out_actions.push_back(action);
        }
    }
}
void InputMapper::MapScalar(const StringId& p_str_id, const ActionDef& p_def,
                            const AxisState& p_axis_state,
                            const DeviceRouting& p_routing,
                            std::vector<ActionEvent>& p_out_actions) const {

    for (InputDeviceId dev_id : p_axis_state.activeDevices()) {
        float value = 0.0f;

        for (const ActionBinding& b : p_def.bindings) {
            if (b.behavior != BindingBehavior::Scalar) {
                continue;
            }

            if (b.source.type == BindingSourceType::Axis) {
                float v = p_axis_state.get(dev_id, b.source.axis);
                if (b.invert) v = -v;
                if (std::abs(v) < b.deadzone) v = 0.0f;
                value += v * b.scale;
            }
        }

        if (value != 0.0f) {
            ActionEvent action{};
            action.action = p_str_id;
            action.type = ActionEventType::Axis1D;
            action.player = p_routing.PlayerFor(dev_id);
            action.x = value;
            p_out_actions.push_back(action);
        }
    }
}

}  // namespace cave
