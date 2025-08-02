#include "asset_registry.h"

#include <fstream>
#include <latch>

#include "engine/algorithm/algorithm.h"
#include "engine/core/string/string_utils.h"
#include "engine/runtime/application.h"
#include "engine/runtime/asset_manager_interface.h"

namespace cave {

namespace fs = std::filesystem;

extern void RegisterAllPersistentAssets(Application* p_app);

auto AssetRegistry::InitializeImpl() -> Result<void> {
    RegisterAllPersistentAssets(m_app);

    fs::path assets_root = fs::path{ m_app->GetResourceFolder() };

    struct Pair {
        bool has_meta;
        bool has_source;
    };

    std::unordered_map<std::string, Pair> resources;

    // go through all files, create meta if not exists
    for (const auto& entry : fs::recursive_directory_iterator(assets_root)) {
        if (entry.is_regular_file()) {
            std::string short_path = m_app->GetAssetManager()->ResolvePath(entry.path());

            auto ext = StringUtils::Extension(short_path);
            if (ext == ".meta") {
                short_path.resize(short_path.size() - 5);  // remove '.meta'
                resources[short_path].has_meta = true;
            } else {
                resources[short_path].has_source = true;
            }
        }
    }

    std::vector<AssetMetaData> assets;
    assets.reserve(resources.size());

    for (const auto& [key, value] : resources) {
        auto meta_path = std::format("{}.meta", key);
        if (value.has_meta) {
            auto res = AssetMetaData::LoadMeta(meta_path);
            if (!res) {
                return CAVE_ERROR(res.error());
            }

            auto meta = std::move(*res);

            if (meta.import_path != key) {
                LOG_WARN("path of asset '{}' is outdated expect: '{}', actual: '{}'", meta.guid.ToString(), meta.import_path, key);
                meta.import_path = key;
            }

            LOG_VERBOSE("'{}' detected, loading...", meta_path);
            assets.emplace_back(std::move(meta));
            continue;
        }

        DEV_ASSERT(value.has_source);
        auto meta = AssetMetaData::CreateMeta(key);
        if (meta.is_none()) {
            // LOG_WARN("file '{}' not supported", key);
            continue;
        }

        auto meta2 = std::move(meta.unwrap_unchecked());
        auto res = meta2.SaveToDisk(nullptr);
        if (!res) {
            return CAVE_ERROR(res.error());
        }

        LOG_VERBOSE("'{}' not detected, creating", meta_path);
        assets.emplace_back(std::move(meta2));
    }

    const int N = static_cast<int>(assets.size());
    std::vector<TopoSortEdge> edges;
    std::unordered_map<Guid, int> mapping;
    for (int i = 0; i < N; ++i) {
        mapping[assets[i].guid] = i;
    }

    for (const auto& asset : assets) {
        auto to = mapping.find(asset.guid);
        DEV_ASSERT(to != mapping.end());
        for (const auto& guid : asset.dependencies) {
            auto from = mapping.find(guid);
            DEV_ASSERT(from != mapping.end());
            edges.push_back({ from->second, to->second });
        }
    }

    const auto order = TopologicalSort(N, edges).unwrap();

    std::latch latch(assets.size());
    for (int idx : order) {
        StartAsyncLoad(std::move(assets[idx]), [](AssetRef, void* p_userdata) {
            DEV_ASSERT(p_userdata);
            std::latch& latch = *reinterpret_cast<std::latch*>(p_userdata);
            latch.count_down(); }, [](void* p_userdata) {
            DEV_ASSERT(p_userdata);
            std::latch& latch = *reinterpret_cast<std::latch*>(p_userdata);
            latch.count_down(); }, &latch);
    }

    latch.wait();
    return Result<void>();
}

void AssetRegistry::FinalizeImpl() {
}

bool AssetRegistry::StartAsyncLoad(AssetMetaData&& p_meta,
                                   AssetLoadSuccessCallback&& p_on_success,
                                   AssetLoadFailureCallback&& p_on_failure,
                                   void* p_userdata) {

    auto entry = std::make_shared<AssetEntry>(std::move(p_meta));
    bool ok = true;
    {
        std::lock_guard<std::mutex> lock(registry_mutex);
        ok = ok && m_guid_map.try_emplace(entry->metadata.guid, entry).second;
        ok = ok && m_path_map.try_emplace(entry->metadata.import_path, entry->metadata.guid).second;
    }
    if (ok) {
        m_app->GetAssetManager()->LoadAssetAsync(entry->metadata.guid,
                                                 std::move(p_on_success),
                                                 std::move(p_on_failure),
                                                 p_userdata);
    }
    return ok;
}

// @TODO: use this for string look up
struct TransparentCompare {
    using is_transparent = void;
    bool operator()(const std::string& lhs, const std::string& rhs) const { return lhs < rhs; }
    bool operator()(const std::string& lhs, const char* rhs) const { return lhs < rhs; }
    bool operator()(const char* lhs, const std::string& rhs) const { return lhs < rhs; }
};

void AssetRegistry::RegisterPersistentAsset(const std::string& p_name,
                                            const Guid& p_guid,
                                            AssetRef p_asset) {
    AssetMetaData meta;
    meta.guid = p_guid;
    meta.type = p_asset->GetType();
    meta.name = p_name;
    meta.import_path = std::format("@persist://{}", p_name);

    std::lock_guard lock(registry_mutex);

    {
        auto [_, ok] = m_path_map.try_emplace(meta.import_path, p_guid);
        DEV_ASSERT(ok);
    }

    {
        std::shared_ptr<AssetEntry> entry = std::make_shared<AssetEntry>(std::move(meta));
        entry->status = AssetStatus::Loaded;
        entry->asset = p_asset;
        auto [_, ok] = m_guid_map.try_emplace(p_guid, std::move(entry));
        DEV_ASSERT(ok);
    }
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
