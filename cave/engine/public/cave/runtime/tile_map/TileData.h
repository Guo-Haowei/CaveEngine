// =============================================================================
// File: cave/runtime/tile_map/TileData.h
// =============================================================================
#pragma once
#include <array>
#include <compare>

#include "cave/core/Option.h"
#include "cave/core/containers/Containers.h"
#include "cave/core/memory/Pointer.h"
#include "cave/runtime/tile_map/TerrainId.h"
#include "cave/runtime/tile_map/TileCoord.h"
#include "cave/runtime/tile_map/TileId.h"

namespace cave {

class ISerializer;
class IDeserializer;

// 32x32 tiles per chunk
constexpr int16_t kTileChunkSize = 32;
constexpr int16_t kTileChunkArea = kTileChunkSize * kTileChunkSize;

TileChunkCoord ToTileChunkCoord(TileCoord coord);
int16_t ToTileLocalX(TileCoord coord);
int16_t ToTileLocalY(TileCoord coord);
TileCoord ToTileCoord(TileChunkCoord chunk_coord, int16_t local_x, int16_t local_y);

struct TileCell {
    TileId tile_id = TileId::invalid();
    TerrainId terrain_id = TerrainId::invalid();

    bool empty() const { return !tile_id.valid() && !terrain_id.valid(); }

    bool hasTile() const { return tile_id.valid(); }

    bool hasTerrain() const { return terrain_id.valid(); }

    void clear() {
        tile_id = {};
        terrain_id = {};
    }

    constexpr bool operator==(const TileCell&) const = default;
};

class TileChunk {
public:
    explicit TileChunk() = default;

    TileCell& at(int16_t local_x, int16_t local_y) {
        return m_local_tiles[local_y * kTileChunkSize + local_x];
    }

    const TileCell& at(int16_t local_x, int16_t local_y) const {
        return m_local_tiles[local_y * kTileChunkSize + local_x];
    }

    bool empty() const;

    std::span<const TileCell> tileData() const { return m_local_tiles; }

private:
    std::array<TileCell, kTileChunkArea> m_local_tiles{};
};

class ChunkedTileData {
public:
    ChunkedTileData() = default;

    ChunkedTileData(const ChunkedTileData& other);

    ChunkedTileData& operator=(const ChunkedTileData& other);

    Option<TileCell> tileAt(TileCoord coord) const;

    bool addTile(TileCoord coord, TileCell cell);

    bool removeTile(TileCoord coord);

    bool addChunk(TileChunkCoord coord, Owner<TileChunk>&& chunk);

    const auto& chunks() const { return m_chunks; }

private:
    HashMap<TileChunkCoord, Owner<TileChunk>> m_chunks;
};

ISerializer& WriteObject(ISerializer& s, const ChunkedTileData& tile_data);

bool ReadObject(IDeserializer& d, ChunkedTileData& tile_data);

}  // namespace cave