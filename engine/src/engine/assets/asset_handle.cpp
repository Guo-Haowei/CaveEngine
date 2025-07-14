#include "asset_handle.h"

#include "asset_entry.h"

namespace my {

bool AssetHandle::IsReady() const {
    auto entry = m_entry.lock();
    return entry && entry->status == AssetStatus::Loaded;
}

IAsset* AssetHandle::Get() const {
    if (auto entry = m_entry.lock(); entry) {
        return entry->asset.get();
    }
    return nullptr;
}

[[nodiscard]] AssetRef AssetHandle::Wait() const {
    auto entry = m_entry.lock();
    DEV_ASSERT(entry);
    return entry->Wait();
}

const AssetMetaData* AssetHandle::GetMeta() const {
    if (auto entry = m_entry.lock(); entry) {
        return &entry->metadata;
    }
    return nullptr;
}

}  // namespace my
