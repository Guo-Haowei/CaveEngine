#include "mesh_renderer_component.h"

#include "engine/assets/material_asset.h"
#include "engine/assets/mesh_asset.h"
#include "engine/core/io/archive.h"
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

void MeshRendererComponent::Serialize(Archive& p_archive, uint32_t) {
    CRASH_NOW();
    unused(p_archive);
    // p_archive.ArchiveValue(flags);
}

void MeshRendererComponent::OnDeserialized() {
    {
        auto handle = AssetRegistry::GetSingleton().FindByGuid<MeshAsset>(m_mesh_id);
        if (handle.is_some()) {
            m_mesh_handle = handle.unwrap_unchecked();
        }
    }

    const size_t mat_count = m_material_ids.size();
    if (mat_count > 0) {
        m_material_handles.resize(mat_count);
    }

    for (size_t i = 0; i < mat_count; ++i) {
        auto handle = AssetRegistry::GetSingleton().FindByGuid<MaterialAsset>(m_material_ids[i]);
        if (handle.is_some()) {
            m_material_handles[i] = handle.unwrap_unchecked();
        }
    }
}

}  // namespace cave
