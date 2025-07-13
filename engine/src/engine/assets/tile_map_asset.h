#pragma once
#include "asset_handle.h"
#include "asset_interface.h"

// clang-format off
namespace YAML { class Node; }
// clang-format on

namespace my {

struct TileMapLayer {
    std::string name;

    Guid tileset;
    int z_index = 0;
    std::unordered_map<uint32_t, int> tiles;

    // Non serialized
    AssetHandle tileset_handle;

    static constexpr uint32_t Pack(int16_t p_x, int16_t p_y) {
        const uint32_t x = static_cast<uint32_t>(p_x);
        const uint32_t y = static_cast<uint32_t>(p_y);
        return x << 16 | y;
    }

    static constexpr std::pair<int16_t, int16_t> Unpack(uint32_t p_key) {
        int16_t a = static_cast<int16_t>(p_key >> 16);
        int16_t b = static_cast<int16_t>(p_key & 0xFFFF);
        return { a, b };
    }

    void AddTile(int16_t p_x, int16_t p_y, int id);
};

class TileMapAsset : public IAsset {
public:
    static constexpr const int VERSION = 0;

    TileMapAsset()
        : IAsset(AssetType::TileMap) {}

    TileMapLayer& AddLayer(std::string&& p_name);

    const auto& GetAllLayers() const { return m_layers; }

    auto SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> override;
    auto LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> override;

    std::vector<Guid> GetDependencies() const override;

private:
    auto LoadFromDiskCurrent(const YAML::Node& p_node) -> Result<void>;

    std::vector<TileMapLayer> m_layers;
};

}  // namespace my
