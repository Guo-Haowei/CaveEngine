// =============================================================================
// File: cave/runtime/script/native/NativeScriptComponent.h
// =============================================================================
#pragma once
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace cave {

struct NativeScriptComponent {
    CAVE_COMPONENT(NativeScriptComponent)

    CAVE_PROP()
    std::string script_id;

    // Non-Serialized
    NativeScript* instance = nullptr;
    bool created = false;
    bool pending_reload = false;
};

}  // namespace cave
