// =============================================================================
// File: cave/runtime/tile_map/TileMapAsset.h
// =============================================================================
#pragma once
#include "TileCoord.h"
#include "TileData.h"

#include "cave/core/reflection/Reflection.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/assets/IAsset.h"

#include "cave/runtime/serialization/Concepts.h"

namespace cave {

class ISerializer;
class IDeserializer;

ISerializer& WriteObject(ISerializer& s, const ChunkedTileData& tile_data);

bool ReadObject(IDeserializer& d, ChunkedTileData& tile_data);

static_assert(Serializable<ChunkedTileData>);

class TileMapAsset : public IAsset {
    CAVE_ASSET(TileMapAsset, AssetType::TileMap, 1)

    CAVE_META(TileMapAsset)

private:
    CAVE_PROP()
    std::string m_name;

    CAVE_PROP(editor = Asset)
    Guid m_tile_set_id;

    CAVE_PROP(editor = Toggle)
    bool m_visible = true;

    CAVE_PROP()
    ChunkedTileData m_tiles;

    // Non serialized
    Handle<TileSetAsset> m_tile_set_handle;
    uint32_t m_revision{ 1 };  // make sure revision is ahead of renderer the first frame

public:
    const Handle<TileSetAsset>& tileSetHandle() const { return m_tile_set_handle; }

    std::string& name() { return m_name; }
    const std::string& name() const { return m_name; }
    void name(std::string&& name) { m_name = std::move(name); }

    const Guid& tileSetGuid() const { return m_tile_set_id; }
    void tileSetGuid(const Guid& guid, bool force = false);

    ChunkedTileData& tiles() { return m_tiles; }
    const ChunkedTileData& tiles() const { return m_tiles; }

    uint32_t revision() const { return m_revision; }
    void incRevision() { ++m_revision; }

    bool visible() const { return m_visible; }
    void visible(bool visible) { m_visible = visible; }

    Result<void> saveToDisk(const AssetMetaData& meta) const override;
    Result<void> loadFromDisk(const AssetMetaData& meta) override;

    Vector<Guid> dependencies() const override;
};

}  // namespace cave
