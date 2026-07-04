#include "AssetRegistry.h"

#include "cave/runtime/framework/IApplication.h"
#include "engine/private/runtime/framework/IAssetManager.h"

#include <fstream>
#include <latch>

namespace cave {

namespace fs = std::filesystem;

extern void RegisterAllPersistentAssets(EngineServices& services);

namespace {

template<typename T>
bool Contains(const std::vector<T>& vec, const T& value) {
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}

template<typename T>
void SortAndUnique(std::vector<T>& vec) {
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}

template<typename T>
void EraseValue(std::vector<T>& vec, const T& value) {
    std::erase(vec, value);
}

}  // namespace

auto AssetRegistry::InitializeImpl() -> Result<void> {
    RegisterAllPersistentAssets(m_app->services());
    return Result<void>();
}

void AssetRegistry::FinalizeImpl() {
}

uint64_t AssetRegistry::startAsyncLoad(AssetMetaData&& meta) {
    auto entry = std::make_shared<AssetEntry>(std::move(meta));
    bool ok = true;
    {
        std::lock_guard<std::mutex> lock(registry_mutex_);
        ok = ok && guid_map_.try_emplace(entry->metadata.guid, entry).second;
        ok = ok && path_map_.try_emplace(entry->metadata.import_path, entry->metadata.guid).second;
    }
    if (ok) {
        return m_app->services().assetManager().submitLoadAsset({ entry->metadata.guid });
    }

    return 0;
}

void AssetRegistry::refreshAllDependencies() {
    std::scoped_lock lock(registry_mutex_);

    deps_.clear();
    reverse_deps_.clear();

    for (const auto& [guid, entry] : guid_map_) {
        refreshDependenciesUnlocked(guid);
    }
}

// @TODO: use this for string look up
struct TransparentCompare {
    using is_transparent = void;
    bool operator()(const std::string& lhs, const std::string& rhs) const { return lhs < rhs; }
    bool operator()(const std::string& lhs, const char* rhs) const { return lhs < rhs; }
    bool operator()(const char* lhs, const std::string& rhs) const { return lhs < rhs; }
};

void AssetRegistry::registerAsset(AssetMetaData&& meta, AssetRef asset) {

    std::lock_guard lock(registry_mutex_);

    Guid guid = meta.guid;
    {
        auto [_, ok] = path_map_.try_emplace(meta.import_path, meta.guid);
        DEV_ASSERT(ok);
    }

    {
        std::shared_ptr<AssetEntry> entry = std::make_shared<AssetEntry>(std::move(meta));
        entry->status = asset ? AssetStatus::Loaded : AssetStatus::Unloaded;
        entry->asset = asset;
        auto [_, ok] = guid_map_.try_emplace(guid, std::move(entry));
        DEV_ASSERT(ok);
    }

    refreshDependenciesUnlocked(guid);
}

void AssetRegistry::registerPersistentAsset(const std::string& name,
                                            const Guid& guid,
                                            AssetRef asset) {
    AssetMetaData meta;
    meta.guid = guid;
    meta.type = asset->type();
    meta.name = name;
    meta.import_path = std::format("@persist://{}", name);

    registerAsset(std::move(meta), asset);
}

Option<AssetHandle> AssetRegistry::findByGuid(const Guid& guid, AssetType type) {
    std::lock_guard lock(registry_mutex_);
    auto it = guid_map_.find(guid);
    if (it != guid_map_.end()) {
        auto ok = type & it->second->metadata.type;
        if (static_cast<bool>(ok)) {
            return Some(AssetHandle(guid, it->second));
        }
    }

    return None();
}

Option<AssetHandle> AssetRegistry::findByPath(const std::string& path, AssetType type) {
    std::lock_guard lock(registry_mutex_);
    auto it = path_map_.find(path);
    if (it != path_map_.end()) {
        const Guid& guid = it->second;
        auto it2 = guid_map_.find(guid);
        if (it2 != guid_map_.end()) {
            auto ok = type & it2->second->metadata.type;
            if (static_cast<bool>(ok)) {
                return Some(AssetHandle(guid, it2->second));
            }
        }
    }

    return None();
}

uint32_t AssetRegistry::revision(const Guid& guid) {
    std::lock_guard lock(registry_mutex_);
    auto it = guid_map_.find(guid);
    if (it != guid_map_.end()) {
        return it->second->revision;
    }
    return 0;
}

void AssetRegistry::moveAsset(std::string old_path, std::string new_path) {
    std::lock_guard lock(registry_mutex_);
    auto it = path_map_.find(old_path);
    DEV_ASSERT(it != path_map_.end());
    const Guid& guid = it->second;

    auto it2 = guid_map_.find(guid);
    DEV_ASSERT(it2 != guid_map_.end());

    path_map_[new_path] = guid;
    it2->second->metadata.import_path = std::move(new_path);

    // @TODO: update mapping
}

bool AssetRegistry::saveAssetHelper(const std::shared_ptr<AssetEntry>& entry) {
    if (!entry->asset) {
        LOG_ERROR("Asset not loaded {}", entry->metadata.import_path);
        return false;
    }

    auto res = entry->asset->saveToDisk(entry->metadata);
    if (!res) {
        LOG_ERROR("{}", ToString(res.error()));
        return false;
    }

    ++entry->revision;
    LOG_OK(LogChannel::Asset,
           "Asset '{}' saved. revision={}",
           entry->metadata.import_path,
           entry->revision);
    return true;
}

bool AssetRegistry::saveAsset(const Guid& guid) {
    std::lock_guard lock(registry_mutex_);

    auto it = guid_map_.find(guid);
    if (it == guid_map_.end()) {
        LOG_ERROR("Asset '{}' not found", guid.toString());
        return false;
    }

    return saveAssetHelper(it->second);
}

std::shared_ptr<AssetEntry> AssetRegistry::entry(const Guid& guid) {
    std::lock_guard lock(registry_mutex_);
    auto it = guid_map_.find(guid);
    if (it == guid_map_.end()) {
        return nullptr;
    }
    return it->second;
}

std::vector<AssetHandle> AssetRegistry::getAssetsOfType(AssetType type) const {
    std::vector<AssetHandle> res;
    std::lock_guard lock(registry_mutex_);
    for (const auto& [guid, entry] : guid_map_) {
        if (entry->metadata.type == type) {
            res.emplace_back(AssetHandle(guid, entry));
        }
    }

    return res;
}

std::vector<Guid> AssetRegistry::findReverseDependencies(Guid dependency) const {
    std::scoped_lock lock(registry_mutex_);

    auto it = reverse_deps_.find(dependency);
    if (it == reverse_deps_.end()) {
        return {};
    }

    return it->second;
}

std::vector<Guid> AssetRegistry::findReverseDependenciesTransitively(Guid dependency) const {
    std::scoped_lock lock(registry_mutex_);

    std::vector<Guid> result;
    std::vector<Guid> stack;
    std::unordered_set<Guid> visited;

    stack.push_back(dependency);
    visited.insert(dependency);

    while (!stack.empty()) {
        Guid current = stack.back();
        stack.pop_back();

        auto it = reverse_deps_.find(current);
        if (it == reverse_deps_.end()) {
            continue;
        }

        for (Guid user : it->second) {
            if (!visited.insert(user).second) {
                continue;
            }

            result.push_back(user);
            stack.push_back(user);
        }
    }

    return result;
}

void AssetRegistry::refreshDependenciesUnlocked(Guid guid) {
    auto it = guid_map_.find(guid);
    if (it == guid_map_.end() || !it->second) {
        return;
    }

    const auto& entry = it->second;

    // Remove old reverse edges.
    auto old_it = deps_.find(guid);
    if (old_it != deps_.end()) {
        for (Guid old_dep : old_it->second) {
            auto rev_it = reverse_deps_.find(old_dep);
            if (rev_it == reverse_deps_.end()) {
                continue;
            }

            EraseValue(rev_it->second, guid);

            if (rev_it->second.empty()) {
                reverse_deps_.erase(rev_it);
            }
        }
    }

    std::vector<Guid> new_deps = entry->asset ? entry->asset->dependencies()
                                              : entry->metadata.dependencies;

    new_deps.erase(
        std::remove_if(new_deps.begin(), new_deps.end(),
                       [&](const Guid& dep) {
                           return dep.isNull() || dep == guid;
                       }),
        new_deps.end());

    SortAndUnique(new_deps);

    deps_[guid] = new_deps;

    entry->metadata.dependencies = new_deps;

    for (Guid dep : new_deps) {
        auto& users = reverse_deps_[dep];

        if (!Contains(users, guid)) {
            users.push_back(guid);
        }
    }
}

}  // namespace cave
