// =============================================================================
// File: engine/private/runtime/input/InputActionMap.cpp
// =============================================================================
#include "InputActionMap.h"

namespace cave {

void InputActionMap::AddAction(const StringId& p_str_id, ActionValueType p_type) {
    m_actions[p_str_id].type = p_type;
}

bool InputActionMap::HasAction(const StringId& p_str_id) const {
    return m_actions.find(p_str_id) != m_actions.end();
}

void InputActionMap::BindDigital(const StringId& p_str_id, Key p_key) {
    auto& def = m_actions[p_str_id];
    ActionBinding binding{};
    binding.behavior = BindingBehavior::Digital;
    binding.source = BindingSource::FromKey(p_key);
    def.bindings.push_back(binding);
}

void InputActionMap::BindScalar(const StringId& p_str_id, Key p_key, float p_scale) {
    auto& def = m_actions[p_str_id];
    ActionBinding binding{};
    binding.behavior = BindingBehavior::Scalar;
    binding.source = BindingSource::FromKey(p_key);
    binding.scale = p_scale;
    def.bindings.push_back(binding);
}

void InputActionMap::BindScalar(const StringId& p_str_id,
                                AxisCode p_axis,
                                float p_scale,
                                float p_deadzone,
                                bool p_invert) {
    auto& def = m_actions[p_str_id];
    ActionBinding binding{};
    binding.behavior = BindingBehavior::Scalar;
    binding.source = BindingSource::FromAxis(p_axis);
    binding.scale = p_scale;
    binding.deadzone = p_deadzone;
    binding.invert = p_invert;
    def.bindings.push_back(binding);
}

const ActionDef* InputActionMap::Find(const StringId& p_str_id) const {
    auto it = m_actions.find(p_str_id);
    return it != m_actions.end() ? &it->second : nullptr;
}

}  // namespace cave
