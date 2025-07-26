#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/math/geomath.h"
#include "engine/reflection/reflection.h"

namespace cave {

struct MaterialComponent {
    CAVE_META(MaterialComponent)

public:
    CAVE_PROP(editor = Color)
    Vector4f base_color = Vector4f::One;

    CAVE_PROP(editor = DragFloat, min = 0.00f, max = 0.99f)
    float metallic = 0.0f;

    CAVE_PROP(editor = DragFloat, min = 0.01f, max = 1)
    float roughness = 1.0f;

    CAVE_PROP(editor = DragFloat, min = 0, max = 1000)
    float emissive = 0.0f;

    CAVE_PROP(editor = Asset)
    Guid m_material_id;

    // Non-serialized
    Handle<MaterialAsset> m_material_handle{};
    std::vector<Handle<ImageAsset>> m_images;

    const Guid& GetResourceGuid() const { return m_material_id; }
    void SetResourceGuid(const Guid& p_guid);

    void OnDeserialized();

private:
    void OnDeserializedHelper(Handle<MaterialAsset>& p_handle);
};

}  // namespace cave
