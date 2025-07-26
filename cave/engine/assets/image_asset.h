#pragma once
#include "asset_interface.h"
#include "engine/renderer/gpu_resource.h"

namespace cave {

struct ImageAsset : IAsset {
    CAVE_ASSET(ImageAsset, AssetType::Image, 0)

    enum class Sampler : uint8_t {
        Linear = 0,
        Point,
        Count,
    };

    enum class ColorSpace : uint8_t {
        Linear = 0,
        SRGB,
        Count,
    };

    PixelFormat format = PixelFormat::UNKNOWN;
    ColorSpace color_space = ColorSpace::Linear;
    Sampler sampler = Sampler::Linear;

    int width = 0;
    int height = 0;
    int num_channels = 0;
    std::vector<uint8_t> buffer;

    // @TODO: write data to meta
    // @TODO: refactor
    std::shared_ptr<GpuTexture> gpu_texture;

    Result<void> LoadFromDisk(const AssetMetaData& p_meta) override;

    Result<void> SaveToDisk(const AssetMetaData& p_meta) const override;

    std::vector<Guid> GetDependencies() const override;
};

DECLARE_ENUM_TRAITS(ImageAsset::Sampler, "linear", "point");
DECLARE_ENUM_TRAITS(ImageAsset::ColorSpace, "linear", "srgb");

}  // namespace cave
