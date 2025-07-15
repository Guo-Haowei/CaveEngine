#pragma once

namespace cave {

#define ASSET_TYPE_LIST            \
    ASSET_TYPE(Image, "image")     \
    ASSET_TYPE(Binary, "binary")   \
    ASSET_TYPE(Text, "text")       \
    ASSET_TYPE(Sprite, "sprite")   \
    ASSET_TYPE(TileMap, "tilemap") \
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

// class AssetType {
// public:
//
//     AssetType(Type type)
//         : m_type(type) {
//     }
//
//     const char* ToString() const;
//
//     bool operator==(const AssetType& p_rhs) const {
//         return m_type == p_rhs.m_type;
//     }
//
//     uint8_t GetData() const { return m_type; }
//
// private:
//     Type m_type;
// };

}  // namespace cave
