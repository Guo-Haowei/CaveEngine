#include "light_component.h"

#include "engine/core/io/archive.h"

namespace cave {

void LightComponent::Serialize(Archive& p_archive, uint32_t p_version) {
    DEV_ASSERT(p_version > 14);

    p_archive.ArchiveValue(m_flags);
    p_archive.ArchiveValue(m_type);
    p_archive.ArchiveValue(m_atten);
    p_archive.ArchiveValue(m_shadowRegion);
}

void LightComponent::OnDeserialized() {
    // @TODO: use common base
    m_flags |= DIRTY;
}

}  // namespace cave
