#include "light_component.h"

namespace cave {

void LightComponent::OnDeserialized() {
    // @TODO: use common base
    m_dirty = true;
}

}  // namespace cave
