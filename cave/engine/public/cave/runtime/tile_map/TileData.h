// =============================================================================
// File: cave/runtime/tile_map/TileData.h
// =============================================================================
#pragma once
#include <array>

#include "cave/core/containers/Containers.h"
#include "cave/runtime/tile_map/TileCoord.h"

namespace cave {

using TileId = uint16_t;
constexpr TileId kEmptyTileId = 0xFFFF;

// 32x32 tiles per chunk
constexpr int16_t kTileChunkSize = 32;
constexpr int16_t kTileChunkArea = kTileChunkSize * kTileChunkSize;

class TileChunk {
public:
    TileChunk();

    TileId& at(int16_t local_x, int16_t local_y) {
        return m_local_tiles[local_y * kTileChunkSize + local_x];
    }

    const TileId& at(int16_t local_x, int16_t local_y) const {
        return m_local_tiles[local_y * kTileChunkSize + local_x];
    }

    bool empty() const;

    std::span<const TileId> tileData() const { return m_local_tiles; }

private:
    std::array<TileId, kTileChunkArea> m_local_tiles{};
};

TileChunkCoord ToTileChunkCoord(TileCoord coord);
int16_t ToTileLocalX(TileCoord coord);
int16_t ToTileLocalY(TileCoord coord);
TileCoord ToTileCoord(TileChunkCoord chunk_coord, int16_t local_x, int16_t local_y);

class ChunkedTileData {
public:
    Option<TileId> tileAt(TileCoord coord) const;

    bool addTile(TileCoord coord, TileId id);

    bool removeTile(TileCoord coord);

    bool addChunk(TileChunkCoord coord, Owner<TileChunk>&& chunk);

    const auto& chunks() const { return m_chunks; }

private:
    HashMap<TileChunkCoord, Owner<TileChunk>> m_chunks;
};

}  // namespace cave