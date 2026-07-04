// =============================================================================
// File: cave/runtime/ecs/components/PrefabInstanceComponent.h
// =============================================================================
#pragma once
#include "cave/core/ids/Guid.h"
#include "cave/core/math/Vector.h"
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave {

class PrefabInstanceComponent {
    CAVE_COMPONENT(PrefabInstanceComponent)

private:
    CAVE_PROP(editor = Asset)
    Guid prefab_id_;

    // Non-serialzed
    ecs::Entity child_;

public:
    ecs::Entity child() const { return child_; }
    void child(ecs::Entity ent) { child_ = ent; }

    const Guid& prefabGuid() const { return prefab_id_; }
    bool SetResourceGuid(const Guid& guid);
};

}  // namespace cave