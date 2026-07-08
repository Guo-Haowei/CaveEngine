#pragma once
#include "cave/core/base/Singleton.h"
#include "cave/runtime/assets/IAsset.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/framework/IService.h"

#include "engine/private/runtime/assets/AssetEntry.h"

namespace cave {

class AssetRegistry : public Singleton<AssetRegistry>, public IService {
public:
    AssetRegistry()
        : IService("AssetRegistry") {}

    Option<AssetHandle> findByGuid(const Guid& guid, AssetType type = AssetType::All);
    Option<AssetHandle> findByPath(const std::string& path, AssetType type = AssetType::All);

    template<AssetClass T>
    Option<Handle<T>> findByPath(const std::string& path) {
        auto handle = findByPath(path, T::kAssetType);
        if (handle.is_none()) return None();
        return Some(Handle<T>(std::move(handle.unwrap_unchecked())));
    }

    template<AssetClass T>
    Option<Handle<T>> findByGuid(const Guid& guid) {
        auto handle = findByGuid(guid, T::kAssetType);
        if (handle.is_none()) return None();
        return Some(Handle<T>(std::move(handle.unwrap_unchecked())));
    }

    uint32_t revision(const Guid& guid);

    void moveAsset(std::string old_path, std::string new_path);

    bool saveAsset(const Guid& guid);

    void registerAsset(AssetMetaData&& meta, AssetRef asset);

    void registerPersistentAsset(const std::string& name,
                                 const Guid& guid,
                                 AssetRef asset);

    std::vector<AssetHandle> getAssetsOfType(AssetType type) const;

    Vector<Guid> findReverseDependencies(Guid dependency) const;
    Vector<Guid> findReverseDependenciesTransitively(Guid dependency) const;

    void refreshAllDependencies();

    // should only used by AssetManager
    //[[deprecated]]
    std::shared_ptr<AssetEntry> entry(const Guid& guid);

    //[[deprecated]]
    uint64_t startAsyncLoad(AssetMetaData&& meta);

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    bool saveAssetHelper(const std::shared_ptr<AssetEntry>& entry);
    void refreshDependenciesUnlocked(Guid guid);

    mutable std::mutex registry_mutex_;
    std::unordered_map<std::string, Guid> path_map_;
    std::unordered_map<Guid, std::shared_ptr<AssetEntry>> guid_map_;

    std::unordered_map<Guid, Vector<Guid>> deps_;
    std::unordered_map<Guid, Vector<Guid>> reverse_deps_;
};

}  // namespace cave
