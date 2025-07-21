#pragma once
#include "asset_meta_data.h"

namespace cave {

struct IAsset;
class AssetEntry;
struct AssetMetaData;

using AssetRef = std::shared_ptr<IAsset>;
using AssetLoadSuccessCallback = void (*)(AssetRef p_asset, void* p_userdata);
using AssetLoadFailureCallback = void (*)(void* p_userdata);

#define CAVE_ASSET(NAME, TYPE, VER)                      \
public:                                                  \
    static inline constexpr AssetType ASSET_TYPE = TYPE; \
    static inline constexpr const int VERSION = VER;     \
    NAME() : IAsset(NAME::ASSET_TYPE) {}

// @TODO: make it a class
struct IAsset {
    const AssetType type;

    IAsset(AssetType p_type)
        : type(p_type) {}

    virtual ~IAsset() = default;

    virtual Result<void> LoadFromDisk(const AssetMetaData&) = 0;

    virtual Result<void> SaveToDisk(const AssetMetaData&) const = 0;

    virtual std::vector<Guid> GetDependencies() const = 0;
};

}  // namespace cave
