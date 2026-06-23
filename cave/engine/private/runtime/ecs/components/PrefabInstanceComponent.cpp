#include "cave/runtime/ecs/components/PrefabInstanceComponent.h"

namespace cave {

bool PrefabInstanceComponent::SetResourceGuid(const Guid& guid) {
    if (guid != prefab_id_) {
        prefab_id_ = guid;
        return true;
    }
    return false;
}

}  // namespace cave
