// =============================================================================
// File: cave/runtime/tile_map/TileId.h
// =============================================================================
#pragma once
#include <compare>
#include <cstdint>

namespace cave {

struct TileId {
    using Type = uint16_t;
    static constexpr Type kEmpty = std::numeric_limits<Type>::max();

    Type value = kEmpty;

    constexpr bool valid() const { return value != kEmpty; }

    constexpr explicit operator bool() const { return valid(); }

    constexpr auto operator<=>(const TileId&) const = default;

    static constexpr TileId invalid() { return {}; }
};

}  // namespace cave
