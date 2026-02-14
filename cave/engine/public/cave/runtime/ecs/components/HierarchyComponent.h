// =============================================================================
// File: engine/public/cave/runtime/ecs/components/HierarchyComponent.h
// =============================================================================
#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/reflection/Reflection.h"

namespace cave {

struct HierarchyComponent {
    CAVE_META(HierarchyComponent)

    CAVE_PROP()
    ecs::Entity parent_id;
};

}  // namespace cave
