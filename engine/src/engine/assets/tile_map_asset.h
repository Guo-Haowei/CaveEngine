#pragma once
#include "asset_handle.h"
#include "asset_interface.h"

// clang-format off
namespace YAML { class Node; }
// clang-format on

namespace my {

// @TODO: make it class once stable
struct TileMapLayer {
    std::string name;

    Guid tileset;
    int z_index = 0;
    std::unordered_map<uint32_t, int> tiles;

    // Non serialized
    AssetHandle tileset_handle;
    uint32_t revision{ 1 };  // start from 1, so it always create the first frame

    static int to_unsigned(int16_t x) {
        return x + 10000;
    }

    static int16_t to_signed(int x) {
        return static_cast<int16_t>(x - 10000);
    }

    static uint32_t Pack(int16_t p_x, int16_t p_y) {
        return (to_unsigned(p_x) << 16) | to_unsigned(p_y);
    }

    static std::pair<int16_t, int16_t> Unpack(uint32_t p_key) {
        uint32_t a = p_key >> 16;
        uint32_t b = p_key & 0xFFFF;
        return { to_signed(a), to_signed(b) };
    }

    void AddTile(int16_t p_x, int16_t p_y, int id);
    void EraseTile(int16_t p_x, int16_t p_y);
};

class TileMapAsset : public IAsset {
public:
    static constexpr const int VERSION = 0;

    TileMapAsset()
        : IAsset(AssetType::TileMap) {}

    TileMapLayer& AddLayer(std::string&& p_name);

    auto& GetAllLayers() { return m_layers; }

    const auto& GetAllLayers() const { return m_layers; }

    auto SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> override;
    auto LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> override;

    std::vector<Guid> GetDependencies() const override;

private:
    auto LoadFromDiskCurrent(const YAML::Node& p_node) -> Result<void>;

    std::vector<TileMapLayer> m_layers;
};

}  // namespace my
