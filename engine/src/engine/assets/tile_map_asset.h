#pragma once
#include "asset_handle.h"
#include "asset_interface.h"

// clang-format off
namespace YAML { class Node; }
// clang-format on

namespace my {

struct TileMapTile {
    uint16_t x;
    uint16_t y;
    uint32_t id;
};

struct TileMapLayer {
    std::string name;

    Guid tileset;
    int z_index = 0;
    std::unordered_map<uint32_t, TileMapTile> tiles;

    // Non serialized
    AssetHandle tileset_handle;
};

class TileMapAsset : public IAsset {
public:
    static constexpr const int VERSION = 0;

    TileMapAsset()
        : IAsset(AssetType::TileMap) {}

    TileMapLayer& AddLayer(const std::string& p_name);

    auto SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> override;
    auto LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> override;

    std::vector<Guid> GetDependencies() const override;

private:
    auto LoadFromDiskCurrent(const YAML::Node& p_node) -> Result<void>;

    std::vector<TileMapLayer> m_layers;
};

}  // namespace my
