#include "mesh_renderer.h"

#include "engine/core/io/archive.h"

namespace cave {

void MeshRendererComponent::SetResourceGuid(const Guid& p_guid) {
    AssetHandle::ReplaceGuidAndHandle(AssetType::Mesh,
                                      p_guid,
                                      m_mesh_id,
                                      m_mesh_handle.RawHandle());
}

void MeshRendererComponent::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(flags);
}

}  // namespace cave
