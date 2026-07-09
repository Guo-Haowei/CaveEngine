#pragma once
#include "engine/private/renderer/gpu_resource.h"
#include "cave/runtime/assets/IAsset.h"

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
    Vector<uint8_t> buffer;

    // @TODO: write data to meta
    // @TODO: refactor
    Ref<GpuTexture> gpu_texture;

    Result<void> loadFromDisk(const AssetMetaData& meta) override;

    Result<void> saveToDisk(const AssetMetaData& meta) const override;

    Vector<Guid> dependencies() const override;
};

DECLARE_ENUM_TRAITS(ImageAsset::Sampler, "linear", "point");
DECLARE_ENUM_TRAITS(ImageAsset::ColorSpace, "linear", "srgb");

}  // namespace cave
