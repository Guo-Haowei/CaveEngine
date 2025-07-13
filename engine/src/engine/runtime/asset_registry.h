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

    std::optional<AssetHandle> Request(const std::string& p_path);

    void MoveAsset(std::string&& p_old, std::string&& p_new);

    void SaveAssets();

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    bool StartAsyncLoad(AssetMetaData&& p_meta,
                        OnAssetLoadSuccessFunc p_on_success,
                        void* p_userdata);

    mutable std::mutex registry_mutex;
    std::unordered_map<std::string, Guid> m_path_map;
    std::unordered_map<Guid, std::shared_ptr<AssetEntry>> m_guid_map;

    friend class AssetManager;
};

}  // namespace my
