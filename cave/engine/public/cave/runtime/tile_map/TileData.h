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

TileChunkCoord ToTileChunkCoord(TileCoord coord);
int16_t ToTileLocalX(TileCoord coord);
int16_t ToTileLocalY(TileCoord coord);
TileCoord ToTileCoord(TileChunkCoord chunk_coord, int16_t local_x, int16_t local_y);

class ChunkedTileData {
public:
    Option<TileId> tileAt(TileCoord coord) const;

    bool addTile(TileCoord coord, TileId id);

    bool removeTile(TileCoord coord);

    auto& chunks() { return chunks_; }
    const auto& chunks() const { return chunks_; }

private:
    std::unordered_map<TileChunkCoord, std::unique_ptr<TileChunk>> chunks_;
};

}  // namespace cave