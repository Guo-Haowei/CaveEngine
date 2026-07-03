#pragma once
#include "cave/runtime/assets/AssetMetaData.h"
#include "cave/runtime/assets/IAsset.h"

namespace cave {

enum class AssetStatus : uint8_t {
    Unloaded,
    Loading,
    Loaded,
    Failed,
};

class AssetEntry {
public:
    AssetMetaData metadata;
    AssetRef asset{ nullptr };
    std::atomic<AssetStatus> status{ AssetStatus::Unloaded };
    std::uint32_t revision = 0;

    AssetEntry(const AssetMetaData& metadata)
        : metadata(metadata) {}

    AssetEntry(AssetMetaData&& metadata)
        : metadata(std::move(metadata)) {}

    [[nodiscard]] AssetRef wait();

    void markLoaded(AssetRef asset);

    void markFailed();

private:
    std::mutex mutex_;
    std::condition_variable cv_;
};

}  // namespace cave
