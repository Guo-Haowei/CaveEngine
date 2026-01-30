#pragma once
#include "engine/private/assets/asset_handle.h"
#include "engine/private/core/math/geomath.h"
#include "cave/core/reflection/Reflection.h"

namespace cave {

struct MaterialComponent {
    CAVE_META(MaterialComponent)

    CAVE_PROP(editor = Color)
    math::Vector4f base_color = math::Vector4f::One;

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
    bool SetResourceGuid(const Guid& p_guid);

    void OnDeserialized();

private:
    void OnDeserializedHelper(Handle<MaterialAsset>& p_handle, bool p_override);
};

}  // namespace cave
