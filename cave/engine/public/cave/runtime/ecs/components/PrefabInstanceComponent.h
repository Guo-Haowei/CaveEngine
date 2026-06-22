// =============================================================================
// File: cave/runtime/ecs/components/PrefabInstanceComponent.h
// =============================================================================
#pragma once
#include "cave/core/ids/Guid.h"
#include "cave/core/math/Vector.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class PrefabInstanceComponent {
    CAVE_COMPONENT(PrefabInstanceComponent)

private:
    CAVE_PROP(editor = Asset)
    Guid prefab_id_;

    CAVE_PROP(editor = Translation)
    math::Vec3f translation_;

public:
    const math::Vec3f& translation() const { return translation_; }
    void translation(const math::Vec3f& translation) { translation_ = translation; }

    const Guid& prefabGuid() const { return prefab_id_; }
    bool SetResourceGuid(const Guid& guid);
};

}  // namespace cave