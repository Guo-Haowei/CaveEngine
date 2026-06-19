// =============================================================================
// File: cave/runtime/tile_map/TileMapAsset.h
// =============================================================================
#pragma once
#include <cstdint>

namespace cave {

struct TileCoord {
    int16_t x, y;

    bool operator==(const TileCoord& rhs) const noexcept {
        return x == rhs.x && y == rhs.y;
    }
};

}  // namespace cave

