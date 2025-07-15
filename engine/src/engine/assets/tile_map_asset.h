#pragma once
#include <nlohmann/json_fwd.hpp>

#include "asset_handle.h"
#include "asset_interface.h"

namespace my {

struct TileIndex {
    int16_t x, y;

    bool operator==(const TileIndex& p_rhs) const noexcept {
        return x == p_rhs.x && y == p_rhs.y;
    }
};

}  // namespace my

namespace std {
template<>
struct hash<::my::TileIndex> {
    std::size_t operator()(const ::my::TileIndex& p_key) const noexcept {
        const uint32_t packed = std::bit_cast<uint32_t>(p_key);
        return std::hash<uint32_t>{}(packed);
    }
};

}  // namespace std

namespace my {

struct TileData {
    uint32_t id;

    explicit TileData(uint32_t p_id = UINT_MAX)
        : id(p_id) {}

    static TileData Empty() {
        return TileData();
    }

    bool operator==(const TileData& p_rhs) const noexcept {
        return id == p_rhs.id;
    }

    bool IsEmpty() const { return *this == Empty(); }
};

enum class TileResult : uint8_t {
    Removed,
    Replaced,
    Added,
    Noop,
};

class TileMapLayer {
    // Non serialized
    AssetHandle m_tileset_handle;
    uint32_t m_revision{ 1 };  // make sure revision is ahead the first frame

public:
    std::string name;
    Guid sprite_guid;
    std::unordered_map<TileIndex, TileData> tiles;
    int z_index = 0;

    TileData GetTile(TileIndex p_index);
    TileResult SetTile(TileIndex p_index, TileData p_new_tile, TileData& p_out_old);

    void SetTile(TileIndex p_index, TileData p_new_tile) {
        TileData dummy;
        SetTile(p_index, p_new_tile, dummy);
    }

    uint32_t GetRevision() const { return m_revision; }
    void IncRevision() { ++m_revision; }
    const auto& GetTiles() const { return tiles; }

    friend class TileMapAsset;
};

class TileMapAsset : public IAsset {
    DECLARE_ASSET(TileMapAsset, AssetType::TileMap)
public:
    static constexpr const int VERSION = 0;

    TileMapLayer& AddLayer(std::string&& p_name);

    auto& GetAllLayers() { return m_layers; }

    const auto& GetAllLayers() const { return m_layers; }

    TileMapLayer* GetLayer(int p_idx) {
        return p_idx < (int)m_layers.size() ? &m_layers[p_idx] : nullptr;
    }

    auto SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> override;
    auto LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> override;

    std::vector<Guid> GetDependencies() const override;

private:
    void LoadFromDiskCurrent(const nlohmann::json& j);

    std::vector<TileMapLayer> m_layers;
};

}  // namespace my
