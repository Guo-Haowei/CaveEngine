// =============================================================================
// File: cave/runtime/assets/AssetMetaData.h
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
    mutable Vector<Guid> dependencies;

    CAVE_PROP()
    std::string source_created_time;

    CAVE_PROP()
    std::string source_last_modified;

    CAVE_PROP()
    mutable std::map<std::string, std::string> import_settings;

    [[nodiscard]] static auto loadMeta(std::string_view path) -> Result<AssetMetaData>;

    [[nodiscard]] static auto createMeta(std::string_view path) -> Option<AssetMetaData>;

    [[nodiscard]] Result<void> saveToDisk(const IAsset* asset) const;
};

}  // namespace cave
