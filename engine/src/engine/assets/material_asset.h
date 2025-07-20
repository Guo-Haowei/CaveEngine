#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/math/geomath.h"
#include "engine/reflection/reflection.h"

namespace cave {

class Archive;

struct MaterialAsset : public IAsset {
    CAVE_ASSET(MaterialAsset, AssetType::Material, 0)

    CAVE_META(MaterialAsset)

    CAVE_PROP(type = color)
    Vector4f baseColor = Vector4f::One;

    CAVE_PROP(type = float)
    float metallic = 0.0f;

    CAVE_PROP(type = float)
    float roughness = 1.0f;

    CAVE_PROP(type = float)
    float emissive = 0.0f;

    // @TODO: do it later
    enum {
        TEXTURE_BASE,
        TEXTURE_NORMAL,
        TEXTURE_METALLIC_ROUGHNESS,
        TEXTURE_MAX,
    };

    struct TextureMap {
        Guid image_id;
        Handle<ImageAsset> handle;
        bool enabled = true;
    };

    std::array<TextureMap, TEXTURE_MAX> textures;

    std::vector<Guid> GetDependencies() const override;

    Result<void> SaveToDisk(const AssetMetaData& p_meta) const override;

    Result<void> LoadFromDisk(const AssetMetaData& p_meta) override;

    void OnDeserialized();
};

}  // namespace cave
