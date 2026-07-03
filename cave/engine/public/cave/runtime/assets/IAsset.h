// =============================================================================
// File: cave/runtime/assets/IAsset.h
// =============================================================================
#pragma once
#include "cave/core/error/Result.h"
#include "cave/runtime/assets/AssetMetaData.h"

namespace cave {

class AssetEntry;
struct AssetMetaData;

#define CAVE_ASSET(NAME, TYPE, VER)                      \
public:                                                  \
    static inline constexpr AssetType ASSET_TYPE = TYPE; \
    static inline constexpr const int VERSION = VER;     \
    AssetType type() const override { return ASSET_TYPE; }

class IAsset {
public:
    virtual ~IAsset() = default;

    virtual AssetType type() const = 0;

    virtual Result<void> loadFromDisk(const AssetMetaData&) = 0;

    virtual Result<void> saveToDisk(const AssetMetaData&) const = 0;

    virtual std::vector<Guid> dependencies() const = 0;
};

using AssetRef = std::shared_ptr<IAsset>;

}  // namespace cave
