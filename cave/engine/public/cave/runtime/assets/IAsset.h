// =============================================================================
// File: cave/runtime/assets/IAsset.h
// =============================================================================
#pragma once
#include "cave/core/containers/Containers.h"
#include "cave/core/error/Result.h"
#include "cave/runtime/assets/AssetMetaData.h"

namespace cave {

class AssetEntry;
struct AssetMetaData;

#define CAVE_ASSET(NAME, TYPE, VER)                      \
public:                                                  \
    static inline constexpr AssetType kAssetType = TYPE; \
    static inline constexpr const int kVersion = VER;    \
    AssetType type() const override { return kAssetType; }

template<typename T>
concept AssetClass = std::derived_from<T, IAsset> &&
                     requires(const T& asset) {
                         { T::kAssetType } -> std::convertible_to<AssetType>;
                         { T::kVersion } -> std::convertible_to<int>;
                         { asset.type() } -> std::same_as<AssetType>;
                     };

class IAsset {
public:
    virtual ~IAsset() = default;

    virtual AssetType type() const = 0;

    virtual Result<void> loadFromDisk(const AssetMetaData&) = 0;

    virtual Result<void> saveToDisk(const AssetMetaData&) const = 0;

    virtual Vector<Guid> dependencies() const = 0;
};

using AssetRef = Ref<IAsset>;

}  // namespace cave
