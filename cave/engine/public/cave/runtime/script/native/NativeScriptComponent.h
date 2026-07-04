// =============================================================================
// File: cave/runtime/script/native/NativeScriptComponent.h
// =============================================================================
#pragma once
#include "cave/core/containers/FixedString.h"
#include "cave/core/variant/Variant.h"
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace cave {

struct NativeScriptComponent {
    CAVE_COMPONENT(NativeScriptComponent)

    CAVE_PROP()
    FixedString<32> name;

    CAVE_PROP(editor = VariantMap)
    VariantMap params;

    // Non-Serialized
    NativeScript* instance = nullptr;
    bool created = false;
    bool pending_reload = false;
};

}  // namespace cave
