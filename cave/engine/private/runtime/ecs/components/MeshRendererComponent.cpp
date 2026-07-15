#include "cave/runtime/ecs/components/MeshRendererComponent.h"

#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

MeshRendererComponent::MeshRendererComponent() {
}

bool MeshRendererComponent::SetResourceGuid(const Guid& p_guid) {
    if (!AssetHandle::replaceGuidAndHandle(AssetType::Mesh,
                                           p_guid,
                                           m_mesh_id,
                                           m_mesh_handle.rawHandle())) {
        return false;
    }

    onDeserialized();
    return true;
}

void MeshRendererComponent::AddMaterial(ecs::Entity material) {
    m_materials.push_back(material);
}

void MeshRendererComponent::onDeserialized() {
    auto handle = AssetRegistry::singleton().findByGuid<MeshAsset>(m_mesh_id);
    if (handle.is_some()) {
        m_mesh_handle = handle.unwrap_unchecked();
    }
}

}  // namespace cave
