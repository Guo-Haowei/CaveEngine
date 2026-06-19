// =============================================================================
// File: cave/runtime/tile_map/TileData.h
// =============================================================================
#pragma once
#include <array>

#include "TileCoord.h"

namespace cave {

using TileId = uint16_t;
constexpr TileId kEmptyTileId = 0xFFFF;

// 32x32 tiles per chunk
constexpr int16_t kTileChunkSize = 32;
constexpr int16_t kTileChunkArea = kTileChunkSize * kTileChunkSize;

struct TileChunk {
    std::array<TileId, kTileChunkArea> tiles{};

    TileChunk() {
        tiles.fill(kEmptyTileId);
    }

    TileId& at(int16_t local_x, int16_t local_y) {
        return tiles[local_y * kTileChunkSize + local_x];
    }

    const TileId& at(int16_t local_x, int16_t local_y) const {
        return tiles[local_y * kTileChunkSize + local_x];
    }

    bool empty() const {
        for (TileId id : tiles) {
            if (id != kEmptyTileId) {
                return false;
            }
        }
        return true;
    }
};

struct TileData {
    std::unordered_map<TileChunkCoord, std::unique_ptr<TileChunk>> chunks;
};

TileChunkCoord ToTileChunkCoord(TileCoord coord);
int16_t ToTileLocalX(TileCoord coord);
int16_t ToTileLocalY(TileCoord coord);
TileCoord ToTileCoord(TileChunkCoord chunk_coord, int16_t local_x, int16_t local_y);

}  // namespace cave