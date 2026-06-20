#include "cave/runtime/tile_map/TileData.h"

namespace cave {

namespace {

int16_t FloorDiv(int16_t a, int16_t b) {
    return (a >= 0) ? (a / b) : ((a - b + 1) / b);
}

int16_t PositiveMod(int16_t value, int16_t divisor) {
    int16_t r = value % divisor;
    if (r < 0) {
        r += divisor;
    }
    return r;
}

}  // namespace

TileChunkCoord ToTileChunkCoord(TileCoord coord) {
    return TileChunkCoord{
        FloorDiv(coord.x, kTileChunkSize),
        FloorDiv(coord.y, kTileChunkSize),
    };
}

int16_t ToTileLocalX(TileCoord coord) {
    return PositiveMod(coord.x, kTileChunkSize);
}

int16_t ToTileLocalY(TileCoord coord) {
    return PositiveMod(coord.y, kTileChunkSize);
}

TileCoord ToTileCoord(TileChunkCoord chunk_coord, int16_t local_x, int16_t local_y) {
    DEV_ASSERT(local_x >= 0 && local_x < kTileChunkSize);
    DEV_ASSERT(local_y >= 0 && local_y < kTileChunkSize);

    return TileCoord{
        static_cast<int16_t>(chunk_coord.x * kTileChunkSize + local_x),
        static_cast<int16_t>(chunk_coord.y * kTileChunkSize + local_y),
    };
}

Option<TileId> ChunkedTileData::tileAt(TileCoord coord) const {
    TileChunkCoord chunk_coord = ToTileChunkCoord(coord);

    auto it = chunks_.find(chunk_coord);
    if (it == chunks_.end()) {
        return None();
    }

    const int16_t x = coord.x - chunk_coord.x * kTileChunkSize;
    const int16_t y = coord.y - chunk_coord.y * kTileChunkSize;
    DEV_ASSERT_INDEX(x, kTileChunkSize);
    DEV_ASSERT_INDEX(y, kTileChunkSize);

    TileId tile = it->second->at(x, y);
    if (tile == kEmptyTileId) {
        return None();
    }
    return Some(tile);
}

bool ChunkedTileData::addTile(TileCoord coord, TileId tile_id) {
    DEV_ASSERT(tile_id != kEmptyTileId);

    TileChunkCoord chunk_coord = ToTileChunkCoord(coord);

    auto& chunk = chunks_[chunk_coord];
    if (chunk == nullptr) {
        chunk = std::make_unique<TileChunk>();
        std::memset(chunk.get(), 0xFFFFFFFF, sizeof(TileChunk));
    }

    const int16_t x = coord.x - chunk_coord.x * kTileChunkSize;
    const int16_t y = coord.y - chunk_coord.y * kTileChunkSize;

    TileId& tile = chunk->at(x, y);
    if (tile == tile_id) {
        return false;
    }

    tile = tile_id;
    return true;
}

bool ChunkedTileData::removeTile(TileCoord coord) {
    TileChunkCoord chunk_coord = ToTileChunkCoord(coord);

    auto it = chunks_.find(chunk_coord);
    if (it == chunks_.end()) {
        return false;
    }

    const int16_t x = coord.x - chunk_coord.x * kTileChunkSize;
    const int16_t y = coord.y - chunk_coord.y * kTileChunkSize;

    TileId& tile = it->second->at(x, y);
    if (tile == kEmptyTileId) {
        return false;
    }

    tile = kEmptyTileId;
    return true;
}

}  // namespace cave
