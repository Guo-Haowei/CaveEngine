#pragma once
#include "asset_type.h"
#include "guid.h"

namespace cave {

struct IAsset;

struct AssetMetaData {
    AssetType type{ AssetType::Unknown };
    Guid guid;
    std::string name;
    std::string path;
    std::vector<Guid> dependencies;

    // @TODO: import settings

    /// Load meta from a .meta file
    [[nodiscard]] static auto LoadMeta(std::string_view p_path) -> Result<AssetMetaData>;

    /// Create meta based on asset file
    [[nodiscard]] static auto CreateMeta(std::string_view p_path) -> Option<AssetMetaData>;

    [[nodiscard]] auto SaveToDisk(const IAsset* p_asset) const -> Result<void>;
};

}  // namespace cave
