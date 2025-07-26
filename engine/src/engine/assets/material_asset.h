#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/math/geomath.h"
#include "engine/reflection/reflection.h"

namespace cave {

enum {
    TEXTURE_BASE,
    TEXTURE_NORMAL,
    TEXTURE_METALLIC_ROUGHNESS,
    TEXTURE_MAX,
};

struct MaterialAsset : public IAsset {
    CAVE_ASSET(MaterialAsset, AssetType::Material, 0)

    CAVE_META(MaterialAsset)
public:
    CAVE_PROP(editor = Color)
    Vector4f base_color = Vector4f::One;

    CAVE_PROP(editor = DragFloat, min = 0.00f, max = 0.99f)
    float metallic = 0.0f;

    CAVE_PROP(editor = DragFloat, min = 0.01f, max = 0.1f)
    float roughness = 1.0f;

    CAVE_PROP(editor = DragFloat, min = 0, max = 1000)
    float emissive = 0.0f;

    CAVE_PROP()
    std::array<Guid, TEXTURE_MAX> textures;

    std::vector<Guid> GetDependencies() const override;

    Result<void> SaveToDisk(const AssetMetaData& p_meta) const override;

    Result<void> LoadFromDisk(const AssetMetaData& p_meta) override;

    // @TODO: fix
    static const MaterialAsset* Default();

    void OnDeserialized();
};

}  // namespace cave
