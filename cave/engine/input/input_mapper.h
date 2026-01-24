#pragma once
#include "engine/input/input_action_map.h"

namespace cave {

class AxisState;
class KeyState;

// @TODO: refactor
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
             const AxisState& p_axis,
             const DeviceRouting& p_routing,
             std::vector<ActionEvent>& p_out_actions) const;

private:
    void MapDigital(const StringId& p_str_id,
                    const ActionDef& p_def,
                    const std::vector<InputEvent>& p_events,
                    const DeviceRouting& p_routing,
                    std::vector<ActionEvent>& p_out_actions) const;

    void MapScalar(const StringId& p_str_id,
                   const ActionDef& p_def,
                   const KeyState& p_keys,
                   const AxisState& p_axis,
                   const DeviceRouting& p_routing,
                   std::vector<ActionEvent>& p_out_actions) const;

    const InputActionMap& m_map;
};

}  // namespace cave
