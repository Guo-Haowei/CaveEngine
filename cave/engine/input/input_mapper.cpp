#include "input_mapper.h"

#include "engine/input/axis_state.h"
#include "engine/input/key_state.h"

namespace cave {

void InputMapper::Map(const std::vector<InputEvent>& p_events,
                      const KeyState& p_key_state,
                      const AxisState& p_axis,
                      const DeviceRouting& p_routing,
                      std::vector<ActionEvent>& p_out_actions) const {
    for (const auto& [name, def] : m_map.GetActions()) {
        switch (def.type) {
            case ActionValueType::Digital: {
                MapDigital(name, def, p_events, p_routing, p_out_actions);
            } break;
            case ActionValueType::Scalar: {
                MapScalar(name, def, p_key_state, p_axis, p_routing, p_out_actions);
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

        for (const auto& b : p_def.bindings) {
            if (b.behavior != BindingBehavior::Digital) {
                continue;
            }
            if (b.source.key != k) {
                continue;
            }

            ActionEvent a{};
            a.action = p_str_id;
            a.type = (e.type == InputEventType::ButtonDown)
                         ? ActionEventType::Pressed
                         : ActionEventType::Released;
            a.player = p_routing.PlayerFor(e.device_id);

            p_out_actions.push_back(a);
            break;
        }
    }
}

void InputMapper::MapScalar(const StringId& p_str_id, const ActionDef& p_def,
                            const KeyState& p_keys,
                            const AxisState& p_axis,
                            const DeviceRouting& p_routing,
                            std::vector<ActionEvent>& p_out_actions) const {
    // Evaluate per device -> per player

    for (InputDeviceId dev_id : p_keys.ActiveDevices()) {
        // Typical editor rule: don’t drive movement while Ctrl/Alt are down
        // (Put your own gating elsewhere if you prefer)
        // if (keys.CtrlDown(dev) || keys.AltDown(dev)) continue;

        float value = 0.0f;

        for (const auto& b : p_def.bindings) {
            if (b.behavior != BindingBehavior::Scalar) {
                continue;
            }

            if (b.source.type == BindingSourceType::Key) {
                if (p_keys.Down(dev_id, b.source.key)) {
                    value += b.scale;
                }
            } else /* if (b.source.type == BindingSourceType::Axis) */ {
                float v = p_axis.Get(dev_id, b.source.axis);
                if (b.invert) v = -v;
                if (std::abs(v) < b.deadzone) v = 0.0f;
                value += v * b.scale;
            }
        }

        if (value != 0.0f) {
            ActionEvent a{};
            a.action = p_str_id;
            a.type = ActionEventType::Axis1D;
            a.player = p_routing.PlayerFor(dev_id);
            a.x = value;
            p_out_actions.push_back(a);
        }
    }
}

}  // namespace cave
