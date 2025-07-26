#pragma once
#include "engine/reflection/reflection.h"

namespace cave {

enum class AssetType : uint32_t {
    Unknown = BIT(0),
    Image = BIT(1),
    Blob = BIT(2),
    SpriteAnimation = BIT(3),
    TileSet = BIT(4),
    TileMap = BIT(5),
    Material = BIT(6),
    Mesh = BIT(7),
    Scene = BIT(8),
    All = 0xFFFFFFFF,
};

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
        for (int i = 0; i < array_length(s_mappings); ++i) {
            if (p_type == s_mappings[i].first) return s_mappings[i].second;
        }
        return "unknown";
    }

    static Option<AssetType> FromString(std::string_view p_val) {
        for (int i = 0; i < array_length(s_mappings); ++i) {
            if (p_val == s_mappings[i].second) return Some(s_mappings[i].first);
        }
        return None();
    }
};

}  // namespace cave
