// =============================================================================
// File: cave/runtime/tile_map/TerrainId.h
// =============================================================================
#pragma once
#include <compare>
#include <cstdint>

#include "cave/core/hash/Hash.h"

namespace cave {

class ISerializer;
class IDeserializer;

struct TerrainId {
    using Type = uint16_t;
    static constexpr Type kEmpty = std::numeric_limits<Type>::max();

    Type value = kEmpty;

    constexpr bool valid() const { return value != kEmpty; }
    constexpr bool isNull() const { return value == kEmpty; }

    constexpr explicit operator bool() const { return valid(); }

    constexpr auto operator<=>(const TerrainId&) const = default;

    static constexpr TerrainId null() { return {}; }
};

ISerializer& WriteObject(ISerializer& s, const TerrainId& id);
bool ReadObject(IDeserializer& d, TerrainId& id);

struct TerrainKey {
    TerrainId terrain_id = TerrainId::null();
    uint16_t mask = 0;

    bool operator==(const TerrainKey&) const = default;
};

}  // namespace cave

template<>
struct std::hash<cave::TerrainKey> {
    size_t operator()(const cave::TerrainKey& key) const noexcept {
        size_t seed = 0;
        cave::Hash::combine(seed, key.terrain_id.value);
        cave::Hash::combine(seed, key.mask);
        return seed;
    }
};
