#pragma once
#include "asset_meta_data.h"

namespace cave {

struct IAsset;
struct BufferAsset;
struct TextAsset;
struct ImageAsset;
class SpriteAsset;
class AssetEntry;
struct AssetMetaData;

using AssetRef = std::shared_ptr<IAsset>;
using AssetLoadSuccessCallback = void (*)(AssetRef p_asset, void* p_userdata);
using AssetLoadFailureCallback = void (*)(void* p_userdata);

#define DECLARE_ASSET(NAME, TYPE)                 \
public:                                           \
    static constexpr AssetType ASSET_TYPE = TYPE; \
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
