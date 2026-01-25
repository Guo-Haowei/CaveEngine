#pragma once
#include <chrono>

#include "engine/assets/asset_type.h"
#include "engine/assets/guid.h"
#include "engine/reflection/meta.h"

namespace cave {

class IAsset;

using TimePoint = std::chrono::system_clock::time_point;

struct AssetMetaData {
    CAVE_META(AssetMetaData)

    CAVE_PROP()
    AssetType type = AssetType::Unknown;

    CAVE_PROP()
    Guid guid;

    CAVE_PROP()
    std::string name;

    CAVE_PROP()
    std::string import_path;

    CAVE_PROP()
    mutable std::vector<Guid> dependencies;

    CAVE_PROP()
    std::string source_created_time;

    CAVE_PROP()
    std::string source_last_modified;

    CAVE_PROP()
    mutable std::map<std::string, std::string> import_settings;

    /// Load meta from a .meta file
    [[nodiscard]] static auto LoadMeta(std::string_view p_path) -> Result<AssetMetaData>;

    /// Create meta based on asset file
    [[nodiscard]] static auto CreateMeta(std::string_view p_path) -> Option<AssetMetaData>;

    [[nodiscard]] Result<void> SaveToDisk(const IAsset* p_asset) const;
};

}  // namespace cave
