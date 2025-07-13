#include "asset_handle.h"

#include "asset_entry.h"

namespace my {

bool AssetHandle::IsReady() const {
    auto entry = m_entry.lock();
    return entry && entry->status == AssetStatus::Loaded;
}

IAsset* AssetHandle::Get() {
    if (auto entry = m_entry.lock(); entry) {
        return entry->asset.get();
    }
    return nullptr;
}

[[nodiscard]] auto AssetHandle::Wait() const -> Result<AssetRef> {
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
