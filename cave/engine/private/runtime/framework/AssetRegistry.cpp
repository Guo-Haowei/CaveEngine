#include "AssetRegistry.h"

#include <fstream>
#include <latch>

#include "engine/private/runtime/framework/Application.h"
#include "engine/private/runtime/framework/IAssetManager.h"

namespace cave {

namespace fs = std::filesystem;

extern void RegisterAllPersistentAssets(Application* p_app);

auto AssetRegistry::InitializeImpl() -> Result<void> {
    RegisterAllPersistentAssets(m_app);
    return Result<void>();
}

void AssetRegistry::FinalizeImpl() {
}

uint64_t AssetRegistry::StartAsyncLoad(AssetMetaData&& p_meta) {
    auto entry = std::make_shared<AssetEntry>(std::move(p_meta));
    bool ok = true;
    {
        std::lock_guard<std::mutex> lock(registry_mutex);
        ok = ok && m_guid_map.try_emplace(entry->metadata.guid, entry).second;
        ok = ok && m_path_map.try_emplace(entry->metadata.import_path, entry->metadata.guid).second;
    }
    if (ok) {
        return m_app->GetAssetManager()->SubmitLoadAsset({ entry->metadata.guid });
    }

    return 0;
}

// @TODO: use this for string look up
struct TransparentCompare {
    using is_transparent = void;
    bool operator()(const std::string& lhs, const std::string& rhs) const { return lhs < rhs; }
    bool operator()(const std::string& lhs, const char* rhs) const { return lhs < rhs; }
    bool operator()(const char* lhs, const std::string& rhs) const { return lhs < rhs; }
};

void AssetRegistry::RegisterAsset(AssetMetaData&& p_meta, AssetRef p_asset) {

    std::lock_guard lock(registry_mutex);

    Guid guid = p_meta.guid;
    {
        auto [_, ok] = m_path_map.try_emplace(p_meta.import_path, p_meta.guid);
        DEV_ASSERT(ok);
    }

    {
        std::shared_ptr<AssetEntry> entry = std::make_shared<AssetEntry>(std::move(p_meta));
        entry->status = p_asset ? AssetStatus::Loaded : AssetStatus::Unloaded;
        entry->asset = p_asset;
        auto [_, ok] = m_guid_map.try_emplace(guid, std::move(entry));
        DEV_ASSERT(ok);
    }
}

void AssetRegistry::RegisterPersistentAsset(const std::string& p_name,
                                            const Guid& p_guid,
                                            AssetRef p_asset) {
    AssetMetaData meta;
    meta.guid = p_guid;
    meta.type = p_asset->GetType();
    meta.name = p_name;
    meta.import_path = std::format("@persist://{}", p_name);

    RegisterAsset(std::move(meta), p_asset);
}

Option<AssetHandle> AssetRegistry::FindByGuid(const Guid& p_guid, AssetType p_type) {
    std::lock_guard lock(registry_mutex);
    auto it = m_guid_map.find(p_guid);
    if (it != m_guid_map.end()) {
        auto ok = p_type & it->second->metadata.type;
        if (static_cast<bool>(ok)) {
            return Some(AssetHandle(p_guid, it->second));
        }
    }

    return None();
}

Option<AssetHandle> AssetRegistry::FindByPath(const std::string& p_path, AssetType p_type) {
    std::lock_guard lock(registry_mutex);
    auto it = m_path_map.find(p_path);
    if (it != m_path_map.end()) {
        const Guid& guid = it->second;
        auto it2 = m_guid_map.find(guid);
        if (it2 != m_guid_map.end()) {
            auto ok = p_type & it2->second->metadata.type;
            if (static_cast<bool>(ok)) {
                return Some(AssetHandle(guid, it2->second));
            }
        }
    }

    return None();
}

void AssetRegistry::MoveAsset(std::string&& p_old, std::string&& p_new) {
    std::lock_guard lock(registry_mutex);
    auto it = m_path_map.find(p_old);
    DEV_ASSERT(it != m_path_map.end());
    const Guid& guid = it->second;

    auto it2 = m_guid_map.find(guid);
    DEV_ASSERT(it2 != m_guid_map.end());

    m_path_map[p_new] = guid;
    it2->second->metadata.import_path = std::move(p_new);
}

bool AssetRegistry::SaveAssetHelper(const std::shared_ptr<AssetEntry>& p_entry) const {
    if (!p_entry->asset) {
        LOG_ERROR("Asset not loaded {}", p_entry->metadata.import_path);
        return false;
    }

    auto res = p_entry->asset->SaveToDisk(p_entry->metadata);
    if (!res) {
        LOG_ERROR("{}", ToString(res.error()));
        return false;
    }

    LOG_OK("Asset '{}' saved!", p_entry->metadata.import_path);
    return true;
}

bool AssetRegistry::SaveAllAssets() const {
    std::lock_guard lock(registry_mutex);
    for (const auto& it : m_guid_map) {
        SaveAssetHelper(it.second);
    }

    return true;
}

bool AssetRegistry::SaveAsset(const Guid& p_guid) const {
    std::lock_guard lock(registry_mutex);

    auto it = m_guid_map.find(p_guid);
    if (it == m_guid_map.end()) {
        LOG_ERROR("Asset '{}' not found", p_guid.ToString());
        return false;
    }

    return SaveAssetHelper(it->second);
}

std::shared_ptr<AssetEntry> AssetRegistry::GetEntry(const Guid& p_guid) {
    std::lock_guard lock(registry_mutex);
    auto it = m_guid_map.find(p_guid);
    if (it == m_guid_map.end()) {
        return nullptr;
    }
    return it->second;
}

std::vector<AssetHandle> AssetRegistry::GetAssetsOfType(AssetType p_type) const {
    std::vector<AssetHandle> res;
    std::lock_guard lock(registry_mutex);
    for (const auto& [guid, entry] : m_guid_map) {
        if (entry->metadata.type == p_type) {
            res.emplace_back(AssetHandle(guid, entry));
        }
    }

    return res;
}

}  // namespace cave
