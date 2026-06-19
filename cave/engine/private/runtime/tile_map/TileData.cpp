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
        chunk_coord.x * kTileChunkSize + local_x,
        chunk_coord.y * kTileChunkSize + local_y,
    };
}

}  // namespace cave
