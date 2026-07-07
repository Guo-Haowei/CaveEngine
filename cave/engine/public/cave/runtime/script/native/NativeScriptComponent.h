// =============================================================================
// File: cave/runtime/script/native/NativeScriptComponent.h
// =============================================================================
#pragma once
#include "cave/core/containers/FixedString.h"
#include "cave/core/variant/Variant.h"
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/runtime/script/native/NativeScriptId.h"

namespace cave {

class NativeScript;

struct NativeScriptComponent {
    CAVE_COMPONENT(NativeScriptComponent)

    CAVE_PROP()
    FixedString<32> name;

    CAVE_PROP(editor = VariantMap)
    VariantMap params;

    // Non-Serialized
    NativeScriptId handle;
    bool always_run_called = false;

    bool operator==(const NativeScriptComponent& rhs) const {
        return name == rhs.name && params == rhs.params;
    }
    bool operator!=(const NativeScriptComponent& rhs) const {
        return name != rhs.name || params != rhs.params;
    }
};

}  // namespace cave
