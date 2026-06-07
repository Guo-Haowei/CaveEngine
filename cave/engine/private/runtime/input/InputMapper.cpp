#include "InputMapper.h"

#include "AxisState.h"
#include "cave/runtime/input/KeyState.h"

namespace cave {

void InputMapper::map(const std::vector<InputEvent>& events,
                      const KeyState& key_state,
                      const AxisState& axis_state,
                      const DeviceRouting& routing,
                      std::vector<ActionEvent>& out_actions) const {
    for (const auto& [name, def] : action_map_.GetActions()) {
        switch (def.type) {
            case ActionValueType::Digital: {
                mapDigital(name, def, events, routing, out_actions);
            } break;
            case ActionValueType::Scalar: {
                mapScalar(name, def, key_state, routing, out_actions);
                mapScalar(name, def, axis_state, routing, out_actions);
            } break;
        }
    }
}

void InputMapper::mapDigital(const StringId& action,
                             const ActionDef& def,
                             const std::vector<InputEvent>& events,
                             const DeviceRouting& routing,
                             std::vector<ActionEvent>& out_actions) const {
    for (const auto& event : events) {
        if (event.consumed) {
            continue;
        }

        if (event.type != InputEventType::ButtonDown && event.type != InputEventType::ButtonUp) {
            continue;
        }

        const Key k = static_cast<Key>(event.code);

        for (const ActionBinding& binding : def.bindings) {
            if (binding.behavior != BindingBehavior::Digital) {
                continue;
            }
            if (binding.source.key != k) {
                continue;
            }

            ActionEvent e{};
            e.action = action;
            e.type = (event.type == InputEventType::ButtonDown)
                         ? ActionEventType::Pressed
                         : ActionEventType::Released;
            e.player = routing.PlayerFor(event.dev_id);

            out_actions.push_back(e);
            break;
        }
    }
}

void InputMapper::mapScalar(const StringId& action, const ActionDef& def,
                            const KeyState& keys,
                            const DeviceRouting& routing,
                            std::vector<ActionEvent>& out_actions) const {

    for (InputDeviceId dev_id : keys.activeDevices()) {
        // Typical editor rule: don't drive movement while Ctrl/Alt are down
        // (Put your own gating elsewhere if you prefer)
        // if (keys.CtrlDown(dev) || keys.AltDown(dev)) continue;

        float value = 0.0f;

        for (const ActionBinding& binding : def.bindings) {
            if (binding.behavior != BindingBehavior::Scalar) {
                continue;
            }

            if (binding.source.type == BindingSourceType::Key) {
                if (keys.down(dev_id, binding.source.key)) {
                    value += binding.scale;
                }
            }
        }

        if (value != 0.0f) {
            ActionEvent e{};
            e.action = action;
            e.type = ActionEventType::Axis1D;
            e.player = routing.PlayerFor(dev_id);
            e.x = value;
            out_actions.push_back(e);
        }
    }
}
void InputMapper::mapScalar(const StringId& action, const ActionDef& def,
                            const AxisState& axis_state,
                            const DeviceRouting& routing,
                            std::vector<ActionEvent>& out_actions) const {

    for (InputDeviceId dev_id : axis_state.activeDevices()) {
        float value = 0.0f;

        for (const ActionBinding& b : def.bindings) {
            if (b.behavior != BindingBehavior::Scalar) {
                continue;
            }

            if (b.source.type == BindingSourceType::Axis) {
                float v = axis_state.get(dev_id, b.source.axis);
                if (b.invert) v = -v;
                if (std::abs(v) < b.deadzone) v = 0.0f;
                value += v * b.scale;
            }
        }

        if (value != 0.0f) {
            ActionEvent e{};
            e.action = action;
            e.type = ActionEventType::Axis1D;
            e.player = routing.PlayerFor(dev_id);
            e.x = value;
            out_actions.push_back(e);
        }
    }
}

}  // namespace cave
