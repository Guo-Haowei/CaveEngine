// =============================================================================
// File: cave/runtime/script/native/NativeScriptComponent.h
// =============================================================================
#pragma once
#include "cave/core/containers/FixedString.h"
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace cave {

struct NativeScriptComponent {
    CAVE_COMPONENT(NativeScriptComponent)

    CAVE_PROP()
    FixedString<32> name;

    // Non-Serialized
    NativeScript* instance = nullptr;
    bool created = false;
    bool pending_reload = false;
};

}  // namespace cave
