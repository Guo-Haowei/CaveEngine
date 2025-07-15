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

using TileData = uint32_t;
inline constexpr TileData TILE_DATA_EMPTY = UINT_MAX;

// @TODO: remove this to TileBrush logic
enum class TileResult : uint8_t {
    Removed,
    Replaced,
    Added,
    Noop,
};

class TileMapLayer {
public:
    TileData GetTile(TileIndex p_index);
    TileResult SetTile(TileIndex p_index, TileData p_new_tile, TileData& p_out_old);

    void SetTile(TileIndex p_index, TileData p_new_tile) {
        TileData dummy;
        SetTile(p_index, p_new_tile, dummy);
    }

    const Handle<SpriteAsset>& GetSpriteHandle() const { return m_sprite_handle; }

    std::string& GetName() { return m_name; }
    const std::string& GetName() const { return m_name; }
    void SetName(std::string&& p_name) { m_name = std::move(p_name); }

    const Guid& GetSpriteGuid() const { return m_sprite_guid; }
    void SetSpriteGuid(const Guid& p_guid);

    const auto& GetTiles() const { return m_tiles; }
    void SetTiles(std::unordered_map<TileIndex, TileData>&& p_tiles);

    uint32_t GetRevision() const { return m_revision; }
    void IncRevision() { ++m_revision; }

    bool IsVisible() const { return m_visible; }
    void SetVisible(bool p_visible) { m_visible = p_visible; }

private:
    std::string m_name;
    Guid m_sprite_guid;
    std::unordered_map<TileIndex, TileData> m_tiles;

    bool m_visible = true;
    int m_z_index = 0;

    // Non serialized
    Handle<SpriteAsset> m_sprite_handle;
    uint32_t m_revision{ 1 };  // make sure revision is ahead the first frame

    friend class TileMapAsset;
};

class TileMapAsset : public IAsset {
    DECLARE_ASSET(TileMapAsset, AssetType::TileMap)
public:
    static constexpr const int VERSION = 0;

    TileMapLayer& AddLayer(std::string&& p_name);

    auto& GetAllLayers() { return m_layers; }

    const auto& GetAllLayers() const { return m_layers; }

    TileMapLayer* GetLayer(uint32_t p_idx) {
        return p_idx < (uint32_t)m_layers.size() ? &m_layers[p_idx] : nullptr;
    }

    auto SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> override;
    auto LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> override;

    std::vector<Guid> GetDependencies() const override;

private:
    void LoadFromDiskCurrent(const nlohmann::json& j);

    std::vector<TileMapLayer> m_layers;
};

}  // namespace my
