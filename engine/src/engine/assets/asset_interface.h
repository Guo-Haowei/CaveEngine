#pragma once
#include "asset_meta_data.h"

namespace my {

struct IAsset;
struct BufferAsset;
struct TextAsset;
struct ImageAsset;
class SpriteSheetAsset;
class AssetEntry;
struct AssetMetaData;

using AssetRef = std::shared_ptr<IAsset>;
using OnAssetLoadSuccessFunc = void (*)(AssetRef p_asset, void* p_userdata);

// @TODO: make it a class
struct IAsset {
    const AssetType type;
    std::weak_ptr<AssetEntry> m_entry;  // to retrieve meta data

    IAsset(AssetType p_type)
        : type(p_type) {}

    virtual ~IAsset() = default;

    virtual auto LoadFromDisk(const AssetMetaData&) -> Result<void> {
        // CRASH_NOW_MSG("TODO: implmenet");
        return Result<void>();
    }

    virtual auto SaveToDisk(const AssetMetaData&) const -> Result<void> {
        // CRASH_NOW_MSG("TODO: implmenet");
        return Result<void>();
    }

    virtual std::vector<Guid> GetDependencies() const {
        return {};
    }
};

}  // namespace my
