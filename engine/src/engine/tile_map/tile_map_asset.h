#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/assets/asset_interface.h"
#include "engine/reflection/reflection.h"
#include "engine/serialization/concept.h"

// @TODO: change tile data to sparsed chunck
// @TODO: change m_sprite_guid to m_tile_set_guid (make tile set asset)

namespace cave {

class ISerializer;
class IDeserializer;

using TileId = uint32_t;

struct TileIndex {
    int16_t x, y;

    bool operator==(const TileIndex& p_rhs) const noexcept {
        return x == p_rhs.x && y == p_rhs.y;
    }
};

struct TileIndexHasher {
    std::size_t operator()(const cave::TileIndex& key) const noexcept {
        const uint32_t packed = std::bit_cast<uint32_t>(key);
        return std::hash<uint32_t>{}(packed);
    }
};

using TileChunk = std::unordered_map<TileIndex, TileId, TileIndexHasher>;

struct TileData {
    TileChunk tiles;
};

ISerializer& WriteObject(ISerializer& p_serializer, const TileData& p_tile_data);

bool ReadObject(IDeserializer& p_deserializer, TileData& p_tile_data);

static_assert(Serializable<TileData>);

class TileMapAsset : public IAsset {
    CAVE_ASSET(TileMapAsset, AssetType::TileMap, 1)

    CAVE_META(TileMapAsset)

    CAVE_PROP(type = string, hint = name)
    std::string m_name;

    CAVE_PROP(type = guid, tooltip = "tileset id")
    Guid m_sprite_guid;

    CAVE_PROP(type = boolean, hint = visibility, tooltip = "toggle layer visibility")
    bool m_is_visible = true;

    CAVE_PROP(type = tile_data)
    TileData m_tiles;

private:
    // Non serialized
    Handle<SpriteAsset> m_sprite_handle;
    uint32_t m_revision{ 1 };  // make sure revision is ahead of renderer the first frame

public:
    Option<TileId> GetTile(TileIndex p_index) const;

    bool AddTile(TileIndex p_index, TileId p_data);

    bool RemoveTile(TileIndex p_index);

    const Handle<SpriteAsset>& GetSpriteHandle() const { return m_sprite_handle; }

    std::string& GetName() { return m_name; }
    const std::string& GetName() const { return m_name; }
    void SetName(std::string&& p_name) { m_name = std::move(p_name); }

    const Guid& GetSpriteGuid() const { return m_sprite_guid; }
    void SetSpriteGuid(const Guid& p_guid, bool p_force = false);

    const auto& GetTiles() const { return m_tiles.tiles; }
    void SetTiles(TileChunk&& p_tiles);

    uint32_t GetRevision() const { return m_revision; }
    void IncRevision() { ++m_revision; }

    bool IsVisible() const { return m_is_visible; }
    void SetVisible(bool p_visible) { m_is_visible = p_visible; }

    auto SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> override;
    auto LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> override;

    std::vector<Guid> GetDependencies() const override {
        return { m_sprite_guid };
    }
};

}  // namespace cave
