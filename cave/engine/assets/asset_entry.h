#pragma once
#include "asset_interface.h"
#include "asset_meta_data.h"

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

    AssetEntry(const AssetMetaData& p_metadata)
        : metadata(p_metadata) {}

    AssetEntry(AssetMetaData&& p_metadata)
        : metadata(std::move(p_metadata)) {}

    [[nodiscard]] AssetRef Wait();

    void MarkLoaded(AssetRef p_asset);

    void MarkFailed();

private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
};

}  // namespace cave
