// =============================================================================
// File: cave/runtime/ecs/components/PrefabInstanceComponent.h
// =============================================================================
#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/Guid.h"
#include "cave/core/math/Vec.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class PrefabInstanceComponent {
    CAVE_COMPONENT(PrefabInstanceComponent)

private:
    CAVE_PROP(editor = Asset)
    Guid m_prefab_id;

public:
    const Guid& prefabGuid() const { return m_prefab_id; }
    void setPrefabGuid(const Guid& guid) { m_prefab_id = guid; }
};

}  // namespace cave