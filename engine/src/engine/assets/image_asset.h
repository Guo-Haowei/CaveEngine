#pragma once
#include "asset_interface.h"
#include "engine/renderer/gpu_resource.h"

namespace cave {

struct ImageAsset : IAsset {
    CAVE_ASSET(ImageAsset, AssetType::Image, 0)

    PixelFormat format = PixelFormat::UNKNOWN;
    int width = 0;
    int height = 0;
    int num_channels = 0;
    std::vector<uint8_t> buffer;

    // @TODO: write data to meta
    // @TODO: refactor
    std::shared_ptr<GpuTexture> gpu_texture;

    auto LoadFromDisk(const AssetMetaData&) -> Result<void> override;

    auto SaveToDisk(const AssetMetaData&) const -> Result<void> override;
};

}  // namespace cave
