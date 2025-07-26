#include "material_component.h"

#include "engine/assets/image_asset.h"
#include "engine/assets/material_asset.h"
#include "engine/runtime/asset_registry.h"

namespace cave {

void MaterialComponent::OnDeserializedHelper(Handle<MaterialAsset>& p_handle) {
    MaterialAsset* mat = p_handle.Get();
    DEV_ASSERT(mat);
    base_color = mat->base_color;
    metallic = mat->metallic;
    roughness = mat->roughness;
    emissive = mat->emissive;

    m_images.reserve(mat->textures.size());
    for (const Guid& guid : mat->textures) {
        m_images.push_back(AssetRegistry::GetSingleton().FindByGuid<ImageAsset>(guid).unwrap_or(Handle<ImageAsset>()));
    }
}

void MaterialComponent::SetResourceGuid(const Guid& p_guid) {
    if (AssetHandle::ReplaceGuidAndHandle(AssetType::Material,
                                          p_guid,
                                          m_material_id,
                                          m_material_handle.RawHandle())) {
        OnDeserializedHelper(m_material_handle);
    }
}

void MaterialComponent::OnDeserialized() {
    m_material_handle = AssetRegistry::GetSingleton().FindByGuid<MaterialAsset>(m_material_id).unwrap();
    OnDeserializedHelper(m_material_handle);
}

}  // namespace cave
