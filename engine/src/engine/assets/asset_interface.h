#pragma once
#include "asset_meta_data.h"

namespace cave {

class AssetEntry;
struct AssetMetaData;

#define CAVE_ASSET(NAME, TYPE, VER)                      \
public:                                                  \
    static inline constexpr AssetType ASSET_TYPE = TYPE; \
    static inline constexpr const int VERSION = VER;     \
    NAME() : IAsset(NAME::ASSET_TYPE) {}

class IAsset {
public:
    IAsset(AssetType p_type)
        : m_type(p_type) {}

    virtual ~IAsset() = default;

    virtual Result<void> LoadFromDisk(const AssetMetaData&) = 0;

    virtual Result<void> SaveToDisk(const AssetMetaData&) const = 0;

    virtual std::vector<Guid> GetDependencies() const = 0;

    AssetType GetType() const { return m_type; }

private:
    AssetType m_type;
};

using AssetRef = std::shared_ptr<IAsset>;
using AssetLoadSuccessCallback = void (*)(AssetRef p_asset, void* p_userdata);
using AssetLoadFailureCallback = void (*)(void* p_userdata);


}  // namespace cave
