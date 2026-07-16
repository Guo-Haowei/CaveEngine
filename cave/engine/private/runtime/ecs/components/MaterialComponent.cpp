#include "cave/runtime/ecs/components/MaterialComponent.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

void MaterialComponent::onDeserializedHelper(Handle<MaterialAsset>& p_handle, bool p_override) {
    MaterialAsset* mat = p_handle.get();
    DEV_ASSERT(mat);
    if (p_override) {
        base_color = mat->base_color;
        metallic = mat->metallic;
        roughness = mat->roughness;
        emissive = mat->emissive;
    }

    m_images.reserve(mat->textures.size());
    for (const Guid& guid : mat->textures) {
        m_images.push_back(AssetRegistry::singleton().findByGuid<ImageAsset>(guid).unwrap_or(Handle<ImageAsset>()));
    }
}

void MaterialComponent::refreshMaterialHandle() {
    if (m_material_id.isNull()) {
        m_material_handle = {};
        return;
    }

    auto handle = AssetRegistry::singleton().findByGuid<MaterialAsset>(m_material_id);
    if (handle.is_some()) {
        m_material_handle = std::move(handle.unwrap_unchecked());
    }
}

void MaterialComponent::onMaterialGuidChanged(const FieldChange& change) {
    DEV_ASSERT((*(const Guid*)(change.old_value)) != (*(const Guid*)(change.new_value)));
    DEV_ASSERT(change.object == this);
    DEV_ASSERT(change.field->id == CAVE_SID("material_id"));

    refreshMaterialHandle();
}

void MaterialComponent::onDeserialized() {
    refreshMaterialHandle();

    if (auto handle = AssetRegistry::singleton().findByGuid<MaterialAsset>(m_material_id)) {
        m_material_handle = handle.unwrap_unchecked();
        onDeserializedHelper(m_material_handle, false);
    }
}

}  // namespace cave
