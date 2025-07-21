#pragma once

namespace cave {

#define ASSET_TYPE_LIST                        \
    ASSET_TYPE(Image, "image")                 \
    ASSET_TYPE(Blob, "blob")                   \
    ASSET_TYPE(SpriteAnimation, "sprite_anim") \
    ASSET_TYPE(TileSet, "tileset")             \
    ASSET_TYPE(TileMap, "tilemap")             \
    ASSET_TYPE(Material, "mat")                \
    ASSET_TYPE(Mesh, "mesh")                   \
    ASSET_TYPE(Scene, "scene")

enum _AssetTypeHelper : uint32_t {
#define ASSET_TYPE(ENUM, ...) _##ENUM##_SHIFT,
    ASSET_TYPE_LIST
#undef ASSET_TYPE
        Count,
};

enum class AssetType : uint32_t {
    Unknown = 0,
#define ASSET_TYPE(ENUM, ...) ENUM = 1 << _AssetTypeHelper::_##ENUM##_SHIFT,
    ASSET_TYPE_LIST
#undef ASSET_TYPE
        All = 0xFFFFFFFF,
};

DEFINE_ENUM_BITWISE_OPERATIONS(AssetType);

const char* ToString(AssetType p_type);

AssetType AssetTypeFromString(std::string_view p_string);

}  // namespace cave
