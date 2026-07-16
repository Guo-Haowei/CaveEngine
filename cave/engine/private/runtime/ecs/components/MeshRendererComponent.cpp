#include "cave/runtime/ecs/components/MeshRendererComponent.h"

#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

MeshRendererComponent::MeshRendererComponent() {
}

void MeshRendererComponent::addMaterial(ecs::Entity material) {
    m_materials.push_back(material);
}

void MeshRendererComponent::refreshMeshHandle() {
    if (m_mesh_id.isNull()) {
        m_mesh_handle = {};
        return;
    }

    auto handle = AssetRegistry::singleton().findByGuid<MeshAsset>(m_mesh_id);
    if (handle.is_some()) {
        m_mesh_handle = handle.unwrap_unchecked();
    }
}

void MeshRendererComponent::onMeshGuidChanged(const FieldChange& change) {
    DEV_ASSERT((*(const Guid*)(change.old_value)) != (*(const Guid*)(change.new_value)));
    DEV_ASSERT(change.object == this);
    DEV_ASSERT(change.field->id == CAVE_SID("mesh_id"));

    refreshMeshHandle();
}

void MeshRendererComponent::onDeserialized() {
    refreshMeshHandle();
}

}  // namespace cave
