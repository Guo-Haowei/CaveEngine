#pragma once
#include "asset_interface.h"
#include "asset_handle.h"
#include "asset_meta_data.h"

#include "engine/math/geomath.h"
#include "engine/renderer/gpu_resource.h"

// clang-format off
namespace YAML { class Node; }
// clang-format on

namespace cave {

struct BufferAsset : IAsset {
    CAVE_ASSET(BufferAsset, AssetType::Binary)

    std::vector<char> buffer;
};

struct TextAsset : IAsset {
    CAVE_ASSET(TextAsset, AssetType::Text)

    std::string source;
};

struct ImageAsset : IAsset {
    CAVE_ASSET(ImageAsset, AssetType::Image)

    PixelFormat format = PixelFormat::UNKNOWN;
    int width = 0;
    int height = 0;
    int num_channels = 0;
    std::vector<uint8_t> buffer;

    // @TODO: write data to meta
    // @TODO: refactor
    std::shared_ptr<GpuTexture> gpu_texture;
};

}  // namespace cave
