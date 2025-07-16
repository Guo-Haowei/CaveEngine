#pragma once
#include <nlohmann/json_fwd.hpp>

#include "engine/assets/asset_handle.h"
#include "engine/assets/asset_interface.h"
#include "engine/reflection/reflection.h"

namespace cave {

struct TileIndex {
    int16_t x, y;

    bool operator==(const TileIndex& p_rhs) const noexcept {
        return x == p_rhs.x && y == p_rhs.y;
    }
};

}  // namespace cave

namespace std {
template<>
struct hash<::cave::TileIndex> {
    std::size_t operator()(const ::cave::TileIndex& p_key) const noexcept {
        const uint32_t packed = std::bit_cast<uint32_t>(p_key);
        return std::hash<uint32_t>{}(packed);
    }
};

}  // namespace std

namespace cave {

using TileId = uint32_t;

using TileData = std::unordered_map<TileIndex, TileId>;

class TileMapAsset : public IAsset {
    CAVE_META(TileMapAsset)

    CAVE_PROP(type = name, editable, serialize)
    std::string m_name;

    CAVE_PROP(type = guid, editable, serialize, tooltip = "tileset id")
    Guid m_sprite_guid;

    CAVE_PROP(type = boolean, editable, serialize, tooltip = "change layer visibility")
    bool m_is_visible;

    CAVE_PROP(serialize)
    TileData m_tiles;

public:
    static constexpr const int VERSION = 1;
    static constexpr AssetType ASSET_TYPE = AssetType::TileMap;

    TileMapAsset()
        : IAsset(AssetType::TileMap)
        , m_is_visible(true) {
    }

    // promote TileMapLayer to TileMapAsset

    Option<TileId> GetTile(TileIndex p_index) const;

    bool AddTile(TileIndex p_index, TileId p_data);

    bool RemoveTile(TileIndex p_index);

    const Handle<SpriteAsset>& GetSpriteHandle() const { return m_sprite_handle; }

    std::string& GetName() { return m_name; }
    const std::string& GetName() const { return m_name; }
    void SetName(std::string&& p_name) { m_name = std::move(p_name); }

    const Guid& GetSpriteGuid() const { return m_sprite_guid; }
    void SetSpriteGuid(const Guid& p_guid);

    const auto& GetTiles() const { return m_tiles; }
    void SetTiles(std::unordered_map<TileIndex, TileId>&& p_tiles);

    uint32_t GetRevision() const { return m_revision; }
    void IncRevision() { ++m_revision; }

    bool IsVisible() const { return m_is_visible; }
    void SetVisible(bool p_visible) { m_is_visible = p_visible; }

    auto SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> override;
    auto LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> override;

    std::vector<Guid> GetDependencies() const override;

private:
    void LoadFromDiskVersion0(const nlohmann::json& j);
    void LoadFromDiskVersion1(const nlohmann::json& j);

    // Non serialized
    Handle<SpriteAsset> m_sprite_handle;
    uint32_t m_revision{ 1 };  // make sure revision is ahead the first frame
};

}  // namespace cave
