#include "MeshRendererComponent.h"

#include "engine/private/assets/material_asset.h"
#include "engine/private/assets/mesh_asset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

MeshRendererComponent::MeshRendererComponent() {
}

bool MeshRendererComponent::SetResourceGuid(const Guid& p_guid) {
    return AssetHandle::ReplaceGuidAndHandle(AssetType::Mesh,
                                             p_guid,
                                             m_mesh_id,
                                             m_mesh_handle.RawHandle());
}

void MeshRendererComponent::AddMaterial(ecs::Entity p_material) {
    m_materials.push_back(p_material);
}

void MeshRendererComponent::OnDeserialized() {
    auto handle = AssetRegistry::GetSingleton().FindByGuid<MeshAsset>(m_mesh_id);
    if (handle.is_some()) {
        m_mesh_handle = handle.unwrap_unchecked();
    }
}

}  // namespace cave
