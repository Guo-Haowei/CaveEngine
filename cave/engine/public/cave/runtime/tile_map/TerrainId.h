// =============================================================================
// File: cave/runtime/tile_map/TerrainId.h
// =============================================================================
#pragma once
#include <compare>
#include <cstdint>

namespace cave {

struct TerrainId {
    using Type = uint16_t;

    Type value = 0;

    constexpr bool valid() const { return value != 0; }

    constexpr explicit operator bool() const { return valid(); }

    constexpr auto operator<=>(const TerrainId&) const = default;

    static constexpr TerrainId invalid() { return {}; }
};

}  // namespace cave
