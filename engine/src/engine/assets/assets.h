#pragma once
#include "asset_interface.h"
#include "asset_handle.h"
#include "asset_meta_data.h"

#include "engine/math/geomath.h"

// clang-format off
namespace YAML { class Node; }
// clang-format on

namespace cave {

struct BufferAsset : IAsset {
    CAVE_ASSET(BufferAsset, AssetType::Binary, 0)

    std::vector<char> buffer;
};

struct TextAsset : IAsset {
    CAVE_ASSET(TextAsset, AssetType::Text, 0)

    std::string source;
};

}  // namespace cave
