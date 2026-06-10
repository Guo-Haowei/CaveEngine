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
    Image           = BIT(0),
    Blob            = BIT(1),
    SpriteAnimation = BIT(2),
    TileSet         = BIT(3),
    TileMap         = BIT(4),
    Material        = BIT(5),
    Mesh            = BIT(6),
    Scene           = BIT(7),
    All             = 0xFFFFFFFF,
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
    };

    static std::string_view ToString(AssetType p_type) {
        for (size_t i = 0; i < std::size(s_mappings); ++i) {
            if (p_type == s_mappings[i].first) return s_mappings[i].second;
        }
        return "unknown";
    }

    static Option<AssetType> FromString(std::string_view p_val) {
        for (size_t i = 0; i < std::size(s_mappings); ++i) {
            if (p_val == s_mappings[i].second) return Some(s_mappings[i].first);
        }
        return None();
    }
};

}  // namespace cave
