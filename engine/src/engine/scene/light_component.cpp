#include "light_component.h"

#include "engine/core/io/archive.h"

namespace cave {

void LightComponent::Serialize(Archive& p_archive, uint32_t) {
    CRASH_NOW();
    // p_archive.ArchiveValue(m_flags);
    p_archive.ArchiveValue(m_type);
    // p_archive.ArchiveValue(m_atten);
    p_archive.ArchiveValue(m_shadow_region);
}

void LightComponent::OnDeserialized() {
    // @TODO: use common base
    m_dirty = true;
}

}  // namespace cave
