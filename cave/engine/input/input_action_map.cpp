#include "input_action_map.h"

namespace cave {

void InputActionMap::AddAction(const StringId& p_str_id, ActionValueType p_type) {
    m_actions[p_str_id].type = p_type;
}

bool InputActionMap::HasAction(const StringId& p_str_id) const {
    return m_actions.find(p_str_id) != m_actions.end();
}

void InputActionMap::BindDigital(const StringId& p_str_id, Key p_key) {
    auto& a = m_actions[p_str_id];
    a.bindings.push_back(ActionBinding{ BindingKind::Digital, p_key, 1.0f });
}

void InputActionMap::BindAxis1D(const StringId& p_str_id, Key p_key, float p_scale) {
    auto& a = m_actions[p_str_id];
    a.bindings.push_back(ActionBinding{ BindingKind::Axis1D, p_key, p_scale });
}

const ActionDef*
InputActionMap::Find(const StringId& p_str_id) const {
    auto it = m_actions.find(p_str_id);
    return it != m_actions.end() ? &it->second : nullptr;
}

}  // namespace cave
