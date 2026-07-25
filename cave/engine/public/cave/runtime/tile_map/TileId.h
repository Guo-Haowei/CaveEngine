// =============================================================================
// File: cave/runtime/tile_map/TileId.h
// =============================================================================
#pragma once
#include <compare>
#include <cstdint>

namespace cave {

class ISerializer;
class IDeserializer;

struct TileId {
    using Type = uint16_t;
    static constexpr Type kEmpty = std::numeric_limits<Type>::max();

    Type value = kEmpty;

    constexpr bool valid() const { return value != kEmpty; }
    constexpr bool isNull() const { return value == kEmpty; }

    constexpr explicit operator bool() const { return valid(); }

    constexpr auto operator<=>(const TileId&) const = default;

    static constexpr TileId null() { return {}; }

    template<typename T>
    static constexpr TileId from(T value) {
        return TileId{ static_cast<Type>(value) };
    }
};

ISerializer& WriteObject(ISerializer& s, const TileId& id);
bool ReadObject(IDeserializer& d, TileId& id);

}  // namespace cave
