#include "mesh_renderer_component.h"

#include "engine/assets/material_asset.h"
#include "engine/assets/mesh_asset.h"
#include "engine/runtime/asset_registry.h"

namespace cave {

MeshRendererComponent::MeshRendererComponent() {
}

void MeshRendererComponent::SetResourceGuid(const Guid& p_guid) {
    AssetHandle::ReplaceGuidAndHandle(AssetType::Mesh,
                                      p_guid,
                                      m_mesh_id,
                                      m_mesh_handle.RawHandle());
}

void MeshRendererComponent::AddMaterial() {
    const size_t old_size = m_materials.size();
    m_materials.resize(old_size + 1);
}

void MeshRendererComponent::OnDeserialized() {
    auto handle = AssetRegistry::GetSingleton().FindByGuid<MeshAsset>(m_mesh_id);
    if (handle.is_some()) {
        m_mesh_handle = handle.unwrap_unchecked();
    }
}

}  // namespace cave
