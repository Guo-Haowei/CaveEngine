#pragma once
#include "engine/input/input_action_map.h"
#include "engine/input/key_state.h"

namespace cave {

class DeviceRouting {
public:
    int PlayerFor(InputDeviceId) const {
        return 0;
    }
};

class InputMapper {
public:
    explicit InputMapper(const InputActionMap& p_map)
        : m_map(p_map) {}

    void Map(const std::vector<InputEvent>& p_events,
             const KeyState& p_keys,
             const DeviceRouting& p_routing,
             std::vector<ActionEvent>& p_out_actions) const;

private:
    void MapDigital(const StringId& p_str_id, const ActionDef& p_def,
                    const std::vector<InputEvent>& p_events,
                    const DeviceRouting& p_routing,
                    std::vector<ActionEvent>& p_out_actions) const;

    void MapAxis1D(const StringId& p_str_id, const ActionDef& p_def,
                   const KeyState& p_keys,
                   const DeviceRouting& p_routing,
                   std::vector<ActionEvent>& p_out_actions) const;

    // Optional convenience: build Axis2D directly from two 1D actions in ActionState instead.
    // If you keep Axis2D in map, implement similarly.
    void MapAxis2D_NotImplementedYet(const StringId&, const ActionDef&,
                                     const KeyState&, const DeviceRouting&,
                                     std::vector<ActionEvent>&) const {}

    const InputActionMap& m_map;
};

}  // namespace cave
