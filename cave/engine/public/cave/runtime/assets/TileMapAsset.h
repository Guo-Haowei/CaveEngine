#pragma once
#include "cave/core/reflection/Reflection.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/assets/IAsset.h"

#include "cave/core/serialization/Concepts.h"

namespace cave {

class ISerializer;
class IDeserializer;

using TileId = uint16_t;
constexpr TileId kEmptyTileId = 0xFFFF;
constexpr int16_t kTileChunkSize = 32;  // 32x32 tiles per chunk

struct TileIndex {
    int16_t x, y;

    bool operator==(const TileIndex& p_rhs) const noexcept {
        return x == p_rhs.x && y == p_rhs.y;
    }
};

struct TileChunk {
    TileId tiles[kTileChunkSize][kTileChunkSize];
};

struct TileIndexHasher {
    std::size_t operator()(const cave::TileIndex& key) const noexcept {
        const uint32_t packed = std::bit_cast<uint32_t>(key);
        return std::hash<uint32_t>{}(packed);
    }
};

struct TileData {
    std::unordered_map<
        TileIndex,
        std::unique_ptr<TileChunk>,
        TileIndexHasher>
        chunks;
};

ISerializer& WriteObject(ISerializer& s, const TileData& p_tile_data);

bool ReadObject(IDeserializer& d, TileData& p_tile_data);

static_assert(Serializable<TileData>);

class TileMapAsset : public IAsset {
    CAVE_ASSET(TileMapAsset, AssetType::TileMap, 1)

    CAVE_META(TileMapAsset)

private:
    CAVE_PROP()
    std::string m_name;

    CAVE_PROP(editor = Asset)
    Guid m_tile_set_id;

    CAVE_PROP(editor = Toggle)
    bool m_visibility = true;

    CAVE_PROP()
    TileData m_tiles;

    // Non serialized
    Handle<TileSetAsset> m_tile_set_handle;
    uint32_t m_revision{ 1 };  // make sure revision is ahead of renderer the first frame

public:
    Option<TileId> tileAt(TileIndex index) const;

    bool addTile(TileIndex index, TileId id);

    bool removeTile(TileIndex index);

    const Handle<TileSetAsset>& tileSetHandle() const { return m_tile_set_handle; }

    std::string& name() { return m_name; }
    const std::string& name() const { return m_name; }
    void name(std::string&& name) { m_name = std::move(name); }

    const Guid& GetTileSetGuid() const { return m_tile_set_id; }
    void SetTileSetGuid(const Guid& guid, bool force = false);

    const TileData& tiles() const { return m_tiles; }

    uint32_t revision() const { return m_revision; }
    void incRevision() { ++m_revision; }

    bool visible() const { return m_visibility; }
    void visible(bool visible) { m_visibility = visible; }

    Result<void> SaveToDisk(const AssetMetaData& meta) const override;
    Result<void> LoadFromDisk(const AssetMetaData& meta) override;

    std::vector<Guid> GetDependencies() const override;

private:
    TileIndex convertIndex(TileIndex index) const;
};

}  // namespace cave
