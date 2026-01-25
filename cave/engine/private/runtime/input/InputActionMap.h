// =============================================================================
// File: engine/private/runtime/input/InputActionMap.h
// =============================================================================
#pragma once
#include "cave/runtime/input/InputTypes.h"

#include "cave/runtime/string/StringId.h"
#include "cave/runtime/input/KeyCode.h"

namespace cave {

enum class BindingBehavior : uint8_t {
    Digital,  // produces Pressed/Released events
    Scalar,   // produces float value each frame
};

enum class BindingSourceType : uint8_t {
    Key,
    Axis,
};

struct BindingSource {
    BindingSourceType type{ BindingSourceType::Key };

    union {
        Key key;
        AxisCode axis;
    };

    BindingSource()
        : key(Key::None) {}

    static BindingSource FromKey(Key k) {
        BindingSource s;
        s.type = BindingSourceType::Key;
        s.key = k;
        return s;
    }

    static BindingSource FromAxis(AxisCode a) {
        BindingSource s;
        s.type = BindingSourceType::Axis;
        s.axis = a;
        return s;
    }
};

struct ActionBinding {
    BindingBehavior behavior{ BindingBehavior::Digital };
    BindingSource source;

    // Scalar modifiers (ignored for Digital)
    float scale{ 1.0f };

    // Axis-only modifiers (ignored for Key sources)
    float deadzone{ 0.0f };
    bool invert{ false };
};

enum class ActionValueType : uint8_t {
    Digital,  // pressed/released
    Scalar,   // float
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
    void BindDigital(const StringId& p_str_id,
                     Key p_key);

    // ScalarButton: key contributes scale while held
    void BindScalar(const StringId& p_str_id,
                    Key p_key,
                    float p_scale);

    // ScalarAxis: axis contributes scale
    void BindScalar(const StringId& p_str_id,
                    AxisCode p_axis,
                    float p_scale,
                    float p_deadzone = 0.0f,
                    bool p_invert = false);

    const ActionDef* Find(const StringId& p_str_id) const;

    const auto& GetActions() const { return m_actions; }

private:
    std::unordered_map<StringId, ActionDef> m_actions;
};

}  // namespace cave
