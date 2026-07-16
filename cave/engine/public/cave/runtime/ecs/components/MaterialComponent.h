// =============================================================================
// File: cave/runtime/ecs/components/MaterialComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Vec.h"

#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

struct MaterialComponent {
    CAVE_COMPONENT(MaterialComponent)

    CAVE_PROP(editor = Color)
    math::Vec4f base_color = math::Vec4f::One;

    CAVE_PROP(editor = DragFloat, min = 0.00f, max = 0.99f)
    float metallic = 0.0f;

    CAVE_PROP(editor = DragFloat, min = 0.01f, max = 1)
    float roughness = 1.0f;

    CAVE_PROP(editor = DragFloat, min = 0, max = 1000)
    float emissive = 0.0f;

    CAVE_PROP(editor = Asset, on_change = onMaterialGuidChanged)
    Guid m_material_id;

    // Non-serialized
    Handle<MaterialAsset> m_material_handle{};
    Vector<Handle<ImageAsset>> m_images;

    const Guid& materialGuid() const { return m_material_id; }

    void onDeserialized();

private:
    void refreshMaterialHandle();

    void onMaterialGuidChanged(const FieldChange& change);

    void onDeserializedHelper(Handle<MaterialAsset>& handle, bool override);
};

}  // namespace cave
