#include "cave/core/reflection/Meta.h"
#include "cave/runtime/ecs/components/PrefabInstanceComponent.h"

#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneSerializer.h"

namespace cave {

void PrefabInstanceComponent::onPrefabGuidChanged(const FieldChange& change) {
    DEV_ASSERT((*(const Guid*)(change.old_value)) != (*(const Guid*)(change.new_value)));
    DEV_ASSERT(change.object == this);
    DEV_ASSERT(change.field->id == CAVE_SID("prefab_id"));

    if (m_prefab_id.isNull()) {
        LOG_ERROR(LogChannel::Scene, "Set prefab_id to null is not allowed");
        return;
    }

    Scene& scene = *change.scene;
    InstantiatePrefab(scene, *this, change.entity);
    scene.rebuildHierarchy();
}

}  // namespace cave
