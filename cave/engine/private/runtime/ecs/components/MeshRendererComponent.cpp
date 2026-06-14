#include "cave/runtime/ecs/components/MeshRendererComponent.h"

#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

MeshRendererComponent::MeshRendererComponent() {
}

bool MeshRendererComponent::SetResourceGuid(const Guid& p_guid) {
    if (!AssetHandle::ReplaceGuidAndHandle(AssetType::Mesh,
                                           p_guid,
                                           m_mesh_id,
                                           m_mesh_handle.RawHandle())) {
        return false;
    }

    OnDeserialized();
    return true;
}

void MeshRendererComponent::AddMaterial(ecs::Entity p_material) {
    m_materials.push_back(p_material);
}

void MeshRendererComponent::OnDeserialized() {
    auto handle = AssetRegistry::singleton().FindByGuid<MeshAsset>(m_mesh_id);
    if (handle.is_some()) {
        m_mesh_handle = handle.unwrap_unchecked();
    }
}

}  // namespace cave
