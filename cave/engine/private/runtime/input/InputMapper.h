#pragma once
#include "InputActionMap.h"

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
    explicit InputMapper(const InputActionMap& map)
        : action_map_(map) {}

    void map(const std::vector<InputEvent>& events,
             const KeyState& key_state,
             const AxisState& axis_state,
             const DeviceRouting& routing,
             std::vector<ActionEvent>& out_actions) const;

private:
    void mapDigital(const StringId& action,
                    const ActionDef& def,
                    const std::vector<InputEvent>& events,
                    const DeviceRouting& routing,
                    std::vector<ActionEvent>& out_actions) const;

    void mapScalar(const StringId& action,
                   const ActionDef& def,
                   const KeyState& key_state,
                   const DeviceRouting& routing,
                   std::vector<ActionEvent>& out_actions) const;

    void mapScalar(const StringId& action,
                   const ActionDef& def,
                   const AxisState& axis_state,
                   const DeviceRouting& routing,
                   std::vector<ActionEvent>& out_actions) const;

    const InputActionMap& action_map_;
};

}  // namespace cave
