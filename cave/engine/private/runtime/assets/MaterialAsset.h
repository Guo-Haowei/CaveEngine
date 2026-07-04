#pragma once
#include "cave/core/math/Vector.h"
#include "cave/core/reflection/Reflection.h"

#include "cave/runtime/assets/AssetHandle.h"

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
    math::Vec4f base_color = math::Vec4f::One;

    CAVE_PROP(editor = DragFloat, min = 0.00f, max = 0.99f)
    float metallic = 0.0f;

    CAVE_PROP(editor = DragFloat, min = 0.01f, max = 0.1f)
    float roughness = 1.0f;

    CAVE_PROP(editor = DragFloat, min = 0, max = 1000)
    float emissive = 0.0f;

    CAVE_PROP()
    std::array<Guid, std::to_underlying(TextureSlot::Count)> textures;

    std::vector<Guid> dependencies() const override;

    Result<void> saveToDisk(const AssetMetaData& p_meta) const override;

    Result<void> loadFromDisk(const AssetMetaData& p_meta) override;

    // @TODO: fix
    static const MaterialAsset* Default();

    void OnDeserialized();
};

}  // namespace cave
