// =============================================================================
// File: cave/runtime/ecs/components/TriggerComponent.h
// =============================================================================
#pragma once
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

struct TriggerComponent {
    CAVE_COMPONENT(TriggerComponent)

    CAVE_PROP(editor = Toggle)
    bool enabled = true;
};

}  // namespace cave
