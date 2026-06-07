#include "InputActionMap.h"

namespace cave {

void InputActionMap::addAction(const StringId& action, ActionValueType type) {
    action_defs_[action].type = type;
}

bool InputActionMap::hasAction(const StringId& action) const {
    return action_defs_.find(action) != action_defs_.end();
}

void InputActionMap::bindDigital(const StringId& action, Key key) {
    auto& def = action_defs_[action];
    ActionBinding binding{};
    binding.behavior = BindingBehavior::Digital;
    binding.source = BindingSource::FromKey(key);
    def.bindings.push_back(binding);
}

void InputActionMap::bindScalar(const StringId& action, Key key, float scale) {
    auto& def = action_defs_[action];
    ActionBinding binding{};
    binding.behavior = BindingBehavior::Scalar;
    binding.source = BindingSource::FromKey(key);
    binding.scale = scale;
    def.bindings.push_back(binding);
}

void InputActionMap::bindScalar(const StringId& action,
                                AxisCode axis,
                                float scale,
                                float deadzone,
                                bool invert) {
    auto& def = action_defs_[action];
    ActionBinding binding{};
    binding.behavior = BindingBehavior::Scalar;
    binding.source = BindingSource::FromAxis(axis);
    binding.scale = scale;
    binding.deadzone = deadzone;
    binding.invert = invert;
    def.bindings.push_back(binding);
}

const ActionDef* InputActionMap::findDef(const StringId& action) const {
    auto it = action_defs_.find(action);
    return it != action_defs_.end() ? &it->second : nullptr;
}

}  // namespace cave
