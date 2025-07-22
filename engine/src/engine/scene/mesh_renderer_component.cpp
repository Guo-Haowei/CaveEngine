#include "mesh_renderer_component.h"

#include "engine/assets/mesh_asset.h"
#include "engine/core/io/archive.h"
#include "engine/runtime/asset_registry.h"

namespace cave {

void MeshRendererComponent::SetResourceGuid(const Guid& p_guid) {
    AssetHandle::ReplaceGuidAndHandle(AssetType::Mesh,
                                      p_guid,
                                      m_mesh_id,
                                      m_mesh_handle.RawHandle());
}

void MeshRendererComponent::Serialize(Archive& p_archive, uint32_t) {
    unused(p_archive);
    // p_archive.ArchiveValue(flags);
}

void MeshRendererComponent::OnDeserialized() {
    auto handle = AssetRegistry::GetSingleton().FindByGuid<MeshAsset>(m_mesh_id);
    if (handle.is_some()) {
        m_mesh_handle = handle.unwrap_unchecked();
    }
}

}  // namespace cave
