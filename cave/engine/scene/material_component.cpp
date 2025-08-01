#include "material_component.h"

#include "engine/assets/image_asset.h"
#include "engine/assets/material_asset.h"
#include "engine/runtime/asset_registry.h"

namespace cave {

void MaterialComponent::OnDeserializedHelper(Handle<MaterialAsset>& p_handle, bool p_override) {
    MaterialAsset* mat = p_handle.Get();
    DEV_ASSERT(mat);
    if (p_override) {
        base_color = mat->base_color;
        metallic = mat->metallic;
        roughness = mat->roughness;
        emissive = mat->emissive;
    }

    m_images.reserve(mat->textures.size());
    for (const Guid& guid : mat->textures) {
        m_images.push_back(AssetRegistry::GetSingleton().FindByGuid<ImageAsset>(guid).unwrap_or(Handle<ImageAsset>()));
    }
}

bool MaterialComponent::SetResourceGuid(const Guid& p_guid) {
    if (AssetHandle::ReplaceGuidAndHandle(AssetType::Material,
                                          p_guid,
                                          m_material_id,
                                          m_material_handle.RawHandle())) {
        OnDeserializedHelper(m_material_handle, true);
        return true;
    }
    return false;
}

void MaterialComponent::OnDeserialized() {
    if (auto handle = AssetRegistry::GetSingleton().FindByGuid<MaterialAsset>(m_material_id); handle.is_some()) {
        m_material_handle = handle.unwrap_unchecked();
        OnDeserializedHelper(m_material_handle, false);
    }
}

}  // namespace cave
