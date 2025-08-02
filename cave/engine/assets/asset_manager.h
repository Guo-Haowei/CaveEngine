#pragma once
#include "engine/runtime/asset_manager_interface.h"

namespace cave {

class AssetManager : public IAssetManager {
public:
    struct LoadTask;

    AssetManager() = default;

    Result<void> InitializeImpl() override;
    void FinalizeImpl() override;

    Result<Guid> CreateAsset(AssetType p_type, const std::filesystem::path& p_folder, const char* p_name = nullptr) override;
    Result<Guid> CreateAsset(AssetType p_type, const std::string& p_short_path) override;

    Result<void> MoveAsset(const std::filesystem::path& p_old, const std::filesystem::path& p_new) override;

    std::string ResolvePath(const std::filesystem::path& p_path) override;

    bool LoadAssetAsync(const Guid& p_guid,
                        AssetLoadSuccessCallback&& p_on_success,
                        AssetLoadFailureCallback&& p_on_failure,
                        void* p_userdata) override;

    bool ImportScene(const std::string& p_path) override;

    static void WorkerMain();
    static void RequestShutdown();

private:
    AssetRef LoadAssetSync(const Guid& p_guid);

    bool EnqueueLoadTask(LoadTask& p_task);

    uint32_t m_fps_counter{ 0 };
    std::mutex m_assetLock;
    std::filesystem::path m_assets_root;
};

}  // namespace cave
