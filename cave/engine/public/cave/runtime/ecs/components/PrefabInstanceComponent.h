// =============================================================================
// File: cave/runtime/ecs/components/PrefabInstanceComponent.h
// =============================================================================
#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/Guid.h"
#include "cave/core/math/Vec.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

struct FieldChange;

class PrefabInstanceComponent {
    CAVE_COMPONENT(PrefabInstanceComponent)

private:
    CAVE_PROP(editor = Asset, on_change = onPrefabGuidChanged)
    Guid m_prefab_id;

    void onPrefabGuidChanged(const FieldChange& change);

public:
    const Guid& prefabGuid() const { return m_prefab_id; }
};

}  // namespace cave