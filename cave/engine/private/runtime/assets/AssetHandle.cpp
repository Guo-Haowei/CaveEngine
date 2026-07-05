#include "cave/runtime/assets/AssetHandle.h"

#include "engine/private/runtime/assets/AssetEntry.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

bool AssetHandle::isReady() const {
    auto entry = m_asset_entry.lock();
    return entry && entry->status == AssetStatus::Loaded;
}

IAsset* AssetHandle::get() const {
    if (auto entry = m_asset_entry.lock(); entry) {
        return entry->asset.get();
    }
    return nullptr;
}

[[nodiscard]] AssetRef AssetHandle::wait() const {
    auto entry = m_asset_entry.lock();
    DEV_ASSERT(entry);
    return entry->wait();
}

AssetMetaData* AssetHandle::meta() {
    if (auto entry = m_asset_entry.lock(); entry) {
        return &entry->metadata;
    }
    return nullptr;
}

const AssetMetaData* AssetHandle::meta() const {
    if (auto entry = m_asset_entry.lock(); entry) {
        return &entry->metadata;
    }
    return nullptr;
}

bool AssetHandle::replaceGuidAndHandle(AssetType type,
                                       const Guid& guid,
                                       Guid& out_id,
                                       AssetHandle& out_handle) {
    if (guid == out_id) {
        return false;
    }

    out_id = guid;

    auto res = AssetRegistry::singleton().findByGuid(guid, type);
    if (res.is_none()) {
        LOG_WARN("asset '{}' not found", guid.toString());
        return false;
    }

    out_handle = std::move(res.unwrap_unchecked());
    return true;
}

}  // namespace cave
