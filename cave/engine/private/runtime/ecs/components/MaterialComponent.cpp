#include "cave/runtime/ecs/components/MaterialComponent.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

void MaterialComponent::OnDeserializedHelper(Handle<MaterialAsset>& p_handle, bool p_override) {
    MaterialAsset* mat = p_handle.get();
    DEV_ASSERT(mat);
    if (p_override) {
        base_color = mat->base_color;
        metallic = mat->metallic;
        roughness = mat->roughness;
        emissive = mat->emissive;
    }

    m_images.reserve(mat->textures.size());
    for (const Guid& guid : mat->textures) {
        m_images.push_back(AssetRegistry::singleton().findByGuid<ImageAsset>(guid).unwrap_or(Handle<ImageAsset>()));
    }
}

bool MaterialComponent::SetResourceGuid(const Guid& p_guid) {
    if (AssetHandle::replaceGuidAndHandle(AssetType::Material,
                                          p_guid,
                                          m_material_id,
                                          m_material_handle.rawHandle())) {
        OnDeserializedHelper(m_material_handle, true);
        return true;
    }
    return false;
}

void MaterialComponent::OnDeserialized() {
    if (auto handle = AssetRegistry::singleton().findByGuid<MaterialAsset>(m_material_id); handle.is_some()) {
        m_material_handle = handle.unwrap_unchecked();
        OnDeserializedHelper(m_material_handle, false);
    }
}

}  // namespace cave
