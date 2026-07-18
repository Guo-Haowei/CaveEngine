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

class TileMapLayer {
    CAVE_META(TileMapLayer)

private:
    CAVE_PROP(editor = InputText)
    std::string m_name;

    CAVE_PROP(editor = Asset)
    Guid m_tile_set_guid;

    CAVE_PROP(editor = Toggle)
    bool m_visible = true;

    CAVE_PROP()
    ChunkedTileData m_chunks;

    // Non serialized
    Handle<TileSetAsset> m_tile_set_handle;

    TileMapLayer(const TileMapLayer&) = delete;
    TileMapLayer& operator=(const TileMapLayer&) = delete;

public:
    TileMapLayer() = default;

    TileMapLayer(TileMapLayer&&) = default;
    TileMapLayer& operator=(TileMapLayer&&) = default;

    void refreshTileSetHandle();

    std::string& name() { return m_name; }
    std::string_view name() const { return m_name; }

    const Guid& tileSetGuid() const { return m_tile_set_guid; }
    void setTileSetGuid(const Guid& guid);

    const auto& handle() const { return m_tile_set_handle; }

    ChunkedTileData& chunks() { return m_chunks; }
    const ChunkedTileData& chunks() const { return m_chunks; }
};

class TileMapAsset : public IAsset {
    CAVE_ASSET(TileMapAsset, AssetType::TileMap, 2)

    CAVE_META(TileMapAsset)

private:
    CAVE_PROP()
    Vector<TileMapLayer> m_layers;

    // Non serialized
    uint32_t m_revision{ 1 };  // make sure revision is ahead of renderer the first frame

public:
    Vector<TileMapLayer>& layers() { return m_layers; }
    std::span<const TileMapLayer> layers() const { return m_layers; }

    uint32_t revision() const { return m_revision; }
    void incRevision() { ++m_revision; }

    Result<void> saveToDisk(const AssetMetaData& meta) const override;
    Result<void> loadFromDisk(const AssetMetaData& meta) override;

    Vector<Guid> dependencies() const override;
};

}  // namespace cave
