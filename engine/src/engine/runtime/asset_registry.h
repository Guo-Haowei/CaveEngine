#pragma once
#include "engine/assets/asset_entry.h"
#include "engine/assets/asset_interface.h"
#include "engine/assets/asset_handle.h"
#include "engine/core/base/singleton.h"
#include "engine/runtime/module.h"

namespace my {

class AssetRegistry : public Singleton<AssetRegistry>, public Module {
public:
    AssetRegistry()
        : Module("AssetRegistry") {}

    std::optional<AssetHandle> FindByGuid(const Guid& p_guid, AssetType p_type = AssetType::Any);
    std::optional<AssetHandle> FindByPath(const std::string& p_path, AssetType p_type = AssetType::Any);

    template<typename T>
    std::optional<Handle<T>> FindByPath(const std::string& p_path) {
        static_assert(requires { T::ASSET_TYPE; }, "T must define static constexpr ASSET_TYPE");
        auto handle = FindByPath(p_path, T::ASSET_TYPE);
        if (!handle) return std::nullopt;
        return Handle<T>(std::move(*handle));
    }

    template<typename T>
    std::optional<Handle<T>> FindByGuid(const Guid& p_guid) {
        static_assert(requires { T::ASSET_TYPE; }, "T must define static constexpr ASSET_TYPE");
        auto handle = FindByGuid(p_guid, T::ASSET_TYPE);
        if (!handle) return std::nullopt;
        return Handle<T>(*std::move(handle));
    }

    void MoveAsset(std::string&& p_old, std::string&& p_new);

    void SaveAssets();

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    bool StartAsyncLoad(AssetMetaData&& p_meta,
                        OnAssetLoadSuccessFunc p_on_success,
                        void* p_userdata);

    std::shared_ptr<AssetEntry> GetEntry(const Guid& p_guid);

    mutable std::mutex registry_mutex;
    std::unordered_map<std::string, Guid> m_path_map;
    std::unordered_map<Guid, std::shared_ptr<AssetEntry>> m_guid_map;

    friend class AssetManager;
};

}  // namespace my
