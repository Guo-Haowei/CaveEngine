// =============================================================================
// File: cave/runtime/assets/AssetType.h
// =============================================================================
#pragma once
#include "cave/core/typedefs.h"
#include "cave/core/math/Utils.h"
#include "cave/core/reflection/Reflection.h"

namespace cave {

// I don't remember why bit flag for asset type

// clang-format off
enum class AssetType : uint32_t {
    Unknown         = 0,

    Image           = 0x00000001,
    Blob            = 0x00000002,
    SpriteAnimation = 0x00000004,
    TileSet         = 0x00000008,
    TileMap         = 0x00000010,
    Material        = 0x00000020,
    Mesh            = 0x00000040,
    Scene           = 0x00000080,
    Prefab          = 0x00000100,

    All             = ~uint32_t{0},
};
// clang-format on

DEFINE_ENUM_BITWISE_OPERATIONS(AssetType);

template<>
struct EnumTraits<AssetType> {
    inline static const std::pair<AssetType, std::string_view> s_mappings[] = {
        { AssetType::Image, "image" },
        { AssetType::Blob, "blob" },
        { AssetType::SpriteAnimation, "sprite_anim" },
        { AssetType::TileSet, "tileset" },
        { AssetType::TileMap, "tilemap" },
        { AssetType::Material, "mat" },
        { AssetType::Mesh, "mesh" },
        { AssetType::Scene, "scene" },
        { AssetType::Prefab, "prefab" },
    };

    static std::string_view ToString(AssetType type) {
        for (size_t i = 0; i < std::size(s_mappings); ++i) {
            if (type == s_mappings[i].first) return s_mappings[i].second;
        }
        return "unknown";
    }

    static Option<AssetType> FromString(std::string_view value) {
        for (size_t i = 0; i < std::size(s_mappings); ++i) {
            if (value == s_mappings[i].second) return Some(s_mappings[i].first);
        }
        return None();
    }
};

}  // namespace cave
