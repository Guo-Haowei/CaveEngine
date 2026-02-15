// =============================================================================
// File: engine/public/cave/runtime/ecs/components/HierarchyComponent.h
// =============================================================================
#pragma once
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave {

struct HierarchyComponent {
    CAVE_COMPONENT(HierarchyComponent)

    CAVE_PROP()
    ecs::Entity parent_id;
};

}  // namespace cave
