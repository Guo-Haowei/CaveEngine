// =============================================================================
// File: cave/runtime/tile_map/TileCoord.h
// =============================================================================
#pragma once
#include <bit>
#include <cstdint>
#include <type_traits>
#include <functional>

namespace cave {

struct TileCoord {
    int16_t x, y;

    bool operator==(const TileCoord&) const = default;

    TileCoord operator+(const TileCoord& rhs) const {
        return TileCoord{ x + rhs.x, y + rhs.y };
    }
};

struct TileChunkCoord {
    int16_t x, y;

    bool operator==(const TileChunkCoord&) const = default;
};

}  // namespace cave

namespace std {

template<>
struct hash<cave::TileCoord> {
    std::size_t operator()(const cave::TileCoord& coord) const {
        const uint32_t packed = std::bit_cast<uint32_t>(coord);
        return std::hash<uint32_t>{}(packed);
    }
};

template<>
struct hash<cave::TileChunkCoord> {
    std::size_t operator()(const cave::TileChunkCoord& coord) const {
        const uint32_t packed = std::bit_cast<uint32_t>(coord);
        return std::hash<uint32_t>{}(packed);
    }
};

}  // namespace std
