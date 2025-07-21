#include "asset_handle.h"

#include "asset_entry.h"

#include "engine/runtime/asset_registry.h"

namespace cave {

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

bool AssetHandle::ReplaceGuidAndHandle(AssetType p_type,
                                       const Guid& p_guid,
                                       Guid& p_out_id,
                                       AssetHandle& p_out_handle) {
    if (p_guid == p_out_id) {
        return false;
    }

    p_out_id = p_guid;

    auto res = AssetRegistry::GetSingleton().FindByGuid(p_guid, p_type);
    if (res.is_none()) {
        LOG_WARN("asset '{}' not found", p_guid.ToString());
        return false;
    }

    p_out_handle = std::move(res.unwrap_unchecked());
    return true;
}

}  // namespace cave
