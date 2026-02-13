// =============================================================================
// File: engine/public/cave/runtime/assets/AssetMetaData.h
// =============================================================================
#pragma once
#include <map>
#include <vector>
#include "cave/core/Option.h"
#include "cave/core/ids/Guid.h"
#include "cave/core/reflection/Meta.h"
#include "cave/runtime/assets/AssetType.h"

namespace cave {

class IAsset;

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
