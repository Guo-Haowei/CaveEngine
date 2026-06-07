// =============================================================================
// File: engine/private/runtime/input/InputActionMap.h
// =============================================================================
#pragma once
#include "cave/core/string/StringId.h"
#include "cave/runtime/input/InputTypes.h"
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
    void addAction(const StringId& action, ActionValueType type);

    bool hasAction(const StringId& action) const;

    void bindDigital(const StringId& action,
                     Key key);

    void bindScalar(const StringId& action,
                    Key key,
                    float scale);

    void bindScalar(const StringId& action,
                    AxisCode axis,
                    float scale,
                    float deadzone = 0.0f,
                    bool invert = false);

    const ActionDef* findDef(const StringId& action) const;

    const auto& GetActions() const { return action_defs_; }

private:
    std::unordered_map<StringId, ActionDef> action_defs_;
};

}  // namespace cave
