// =============================================================================
// File: cave/runtime/ecs/components/HierarchyComponent.h
// =============================================================================
#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

struct HierarchyComponent {
    CAVE_COMPONENT(HierarchyComponent)

    CAVE_PROP()
    ecs::Entity parent_id;
};

}  // namespace cave
