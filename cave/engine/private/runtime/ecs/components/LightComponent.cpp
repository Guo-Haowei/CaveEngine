#include "cave/runtime/ecs/components/LightComponent.h"

namespace cave {

void LightComponent::onDeserialized() {
    // @TODO: use common base
    m_dirty = true;
}

}  // namespace cave
