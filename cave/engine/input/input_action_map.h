#pragma once
#include "engine/core/string/string_id.h"
#include "engine/input/input_types.h"
#include "engine/input/key_code.h"

namespace cave {

enum class BindingKind : uint8_t {
    Digital,
    Axis1D
};

struct ActionBinding {
    BindingKind kind{ BindingKind::Digital };
    Key key{ Key::None };
    float scale = 1.0f;  // For Axis1D: add this when key is held (e.g. left=-1, right=+1)
};

enum class ActionValueType : uint8_t {
    Digital,
    Axis1D,
    Axis2D
};

struct ActionDef {
    ActionValueType type;
    std::vector<ActionBinding> bindings;
};

class InputActionMap {
public:
    void AddAction(const StringId& p_str_id, ActionValueType p_type);

    bool HasAction(const StringId& p_str_id) const;

    // Digital: key press/release generates events
    void BindDigital(const StringId& p_str_id, Key p_key);

    // Axis1D: key contributes scale while held
    void BindAxis1D(const StringId& p_str_id, Key p_key, float p_scale);

    const ActionDef* Find(const StringId& p_str_id) const;

    const auto& GetActions() const { return m_actions; }

private:
    std::unordered_map<StringId, ActionDef> m_actions;
};

}  // namespace cave
