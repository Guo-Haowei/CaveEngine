#include "cave/core/reflection/Meta.h"
#include "cave/runtime/ecs/components/HierarchyComponent.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using ::cave::ecs::Entity;

void HierarchyComponent::onParentChanged(const FieldChange& change) {
    const Entity old_parent =
        *static_cast<const Entity*>(change.old_value);

    const Entity new_parent =
        *static_cast<const Entity*>(change.new_value);

    change.scene->hierarchy().onParentChanged(
        change.entity,
        old_parent,
        new_parent);
}

}  // namespace cave
