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
    std::weak_ptr<AssetEntry> m_entry;

    IAsset(AssetType p_type)
        : type(p_type) {}

    virtual ~IAsset() = default;

    [[nodiscard]] virtual auto LoadFromDisk(const AssetMetaData&) -> Result<void> {
        // CRASH_NOW_MSG("TODO: implmenet");
        return Result<void>();
    }

    [[nodiscard]] virtual auto SaveToDisk(const AssetMetaData&) const -> Result<void> {
        // CRASH_NOW_MSG("TODO: implmenet");
        return Result<void>();
    }

    [[nodiscard]] virtual std::vector<Guid> GetDependencies() const {
        return {};
    }
};

}  // namespace cave
