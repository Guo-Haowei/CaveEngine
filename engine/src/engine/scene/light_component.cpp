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

#if 0
void LightComponent::Attenuation::RegisterClass() {
    BEGIN_REGISTRY(LightComponent::Attenuation);
    REGISTER_FIELD(LightComponent::Attenuation, "constant", constant);
    REGISTER_FIELD(LightComponent::Attenuation, "linear", linear);
    REGISTER_FIELD(LightComponent::Attenuation, "quadratic", quadratic);
    END_REGISTRY(LightComponent::Attenuation);
}

void LightComponent::RegisterClass() {
    BEGIN_REGISTRY(LightComponent);
    REGISTER_FIELD(LightComponent, "flags", m_flags);
    REGISTER_FIELD(LightComponent, "type", m_type);
    REGISTER_FIELD(LightComponent, "shadow_region", m_shadowRegion, FieldFlag::NUALLABLE);
    REGISTER_FIELD(LightComponent, "attenuation", m_atten, FieldFlag::NUALLABLE);
    END_REGISTRY(LightComponent);
}
#endif

}  // namespace cave
