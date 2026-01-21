#pragma once
#include "engine/runtime/asset_manager_interface.h"

namespace cave {

class AssetManager : public IAssetManager {
public:
    AssetManager() = default;

    Result<void> InitializeImpl() override;
    void FinalizeImpl() override;

    void Update() override {}

    Result<Guid> CreateAsset(AssetType p_type, const std::filesystem::path& p_folder, const char* p_name = nullptr) override;
    Result<Guid> CreateAsset(AssetType p_type, const std::string& p_short_path) override;

    Result<void> MoveAsset(const std::filesystem::path& p_old, const std::filesystem::path& p_new) override;

    // @TODO: add these interfaces
    uint64_t SubmitLoadAssets(const AssetLoadRequest& p_request) override;

    uint64_t SubmitImportScene(const SceneImportRequest& p_request) override;

    // @TODO: deprecate
    [[nodiscard]] std::string ResolvePath(const std::filesystem::path& p_path) override;

    // @TODO: deprecate
    bool LoadAssetAsync(const Guid& p_guid) override;

    // @TODO: deprecate
    AssetRef LoadAssetSync(const Guid& p_guid) override;

    // @TODO: deprecate
    bool ImportSceneAsync(const std::filesystem::path& p_source_path,
                          const std::filesystem::path& p_dest_dir) override;

protected:
    uint32_t m_fps_counter{ 0 };
    std::mutex m_asset_lock;
};

}  // namespace cave
