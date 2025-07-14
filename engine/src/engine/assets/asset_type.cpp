#include "asset_type.h"

namespace my {

static_assert(std::to_underlying(AssetType::Image) == 1);
static_assert(std::to_underlying(AssetType::Binary) == 2);
static_assert(std::to_underlying(AssetType::Text) == 4);

const char* ToString(AssetType p_type) {
    switch (p_type) {
#define ASSET_TYPE(ENUM, NAME) \
    case AssetType::ENUM:      \
        return NAME;
        ASSET_TYPE_LIST
#undef ASSET_TYPE
        default:
            CRASH_NOW();
            return nullptr;
    }
}

AssetType AssetTypeFromString(std::string_view p_string) {
#define ASSET_TYPE(ENUM, NAME) \
    if (p_string == NAME) { return AssetType::ENUM; }
    ASSET_TYPE_LIST
#undef ASSET_TYPE
    return AssetType::Unknown;
}

}  // namespace my
