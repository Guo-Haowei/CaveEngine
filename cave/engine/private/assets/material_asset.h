#pragma once
#include "engine/private/assets/asset_handle.h"
#include "engine/private/core/math/geomath.h"
#include "cave/core/reflection/Reflection.h"

namespace cave {

enum class TextureSlot {
    Base,
    Normal,
    MetallicRoughness,
    Count,
};

DECLARE_ENUM_TRAITS(TextureSlot, "base", "normal", "material");

struct MaterialAsset : public IAsset {
    CAVE_ASSET(MaterialAsset, AssetType::Material, 0)

    CAVE_META(MaterialAsset)

    CAVE_PROP(editor = Color)
    math::Vector4f base_color = math::Vector4f::One;

    CAVE_PROP(editor = DragFloat, min = 0.00f, max = 0.99f)
    float metallic = 0.0f;

    CAVE_PROP(editor = DragFloat, min = 0.01f, max = 0.1f)
    float roughness = 1.0f;

    CAVE_PROP(editor = DragFloat, min = 0, max = 1000)
    float emissive = 0.0f;

    CAVE_PROP()
    std::array<Guid, std::to_underlying(TextureSlot::Count)> textures;

    std::vector<Guid> GetDependencies() const override;

    Result<void> SaveToDisk(const AssetMetaData& p_meta) const override;

    Result<void> LoadFromDisk(const AssetMetaData& p_meta) override;

    // @TODO: fix
    static const MaterialAsset* Default();

    void OnDeserialized();
};

}  // namespace cave
