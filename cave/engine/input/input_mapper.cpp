#include "input_mapper.h"

namespace cave {

void InputMapper::Map(const std::vector<InputEvent>& p_events,
                      const KeyState& p_key_state,
                      const DeviceRouting& p_routing,
                      std::vector<ActionEvent>& p_out_actions) const {
    for (const auto& [name, def] : m_map.GetActions()) {
        switch (def.type) {
            case ActionValueType::Digital: {
                MapDigital(name, def, p_events, p_routing, p_out_actions);
            } break;
            case ActionValueType::Axis1D: {
                MapAxis1D(name, def, p_key_state, p_routing, p_out_actions);
            } break;
            case ActionValueType::Axis2D: {
                // Recommended approach:
                // build 2D vectors via ActionState::GetVector from four 1D actions.
                // If you insist on direct Axis2D bindings, implement here.
            } break;
        }
    }
}

void InputMapper::MapDigital(const StringId& p_str_id, const ActionDef& p_def,
                             const std::vector<InputEvent>& p_events,
                             const DeviceRouting& p_routing,
                             std::vector<ActionEvent>& p_out_actions) const {
    for (const auto& e : p_events) {
        if (e.consumed) {
            continue;
        }

        if (e.type != InputEventType::ButtonDown &&
            e.type != InputEventType::ButtonUp) {
            continue;
        }

        const Key k = static_cast<Key>(e.code);

        for (const auto& b : p_def.bindings) {
            if (b.kind != BindingKind::Digital) {
                continue;
            }
            if (b.key != k) {
                continue;
            }

            ActionEvent a{};
            a.action = p_str_id;
            a.type = (e.type == InputEventType::ButtonDown)
                         ? ActionEventType::Pressed
                         : ActionEventType::Released;
            a.player = p_routing.PlayerFor(e.device);

            p_out_actions.push_back(a);
            break;
        }
    }
}

void InputMapper::MapAxis1D(const StringId& p_str_id, const ActionDef& p_def,
                            const KeyState& p_keys,
                            const DeviceRouting& p_routing,
                            std::vector<ActionEvent>& p_out_actions) const {
    // Evaluate per device -> per player

    for (InputDeviceId dev : p_keys.ActiveDevices()) {
        // Typical editor rule: don’t drive movement while Ctrl/Alt are down
        // (Put your own gating elsewhere if you prefer)
        // if (keys.CtrlDown(dev) || keys.AltDown(dev)) continue;

        float value = 0.0f;

        for (const auto& b : p_def.bindings) {
            if (b.kind != BindingKind::Axis1DKey) {
                continue;
            }

            if (p_keys.Down(dev, b.key)) {
                value += b.scale;
            }
        }

        if (value != 0.0f) {
            ActionEvent a{};
            a.action = p_str_id;
            a.type = ActionEventType::Axis1D;
            a.player = p_routing.PlayerFor(dev);
            a.x = value;
            p_out_actions.push_back(a);
        }
    }
}

}  // namespace cave
