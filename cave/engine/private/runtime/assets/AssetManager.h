#pragma once
#include "engine/private/runtime/framework/IAssetManager.h"

namespace cave {

struct EngineServices;

class AssetManager : public IAssetManager {
public:
    AssetManager() = default;

    Result<void> InitializeImpl() override;
    void FinalizeImpl() override;

    void update() override {}

    Result<Guid> createAsset(AssetType type,
                             const std::filesystem::path& folder,
                             const char* name = nullptr) override;

    Result<Guid> createAsset(AssetType type,
                             const std::string& short_path) override;

    Result<Guid> exportPrefab(const Scene& scene, ecs::Entity root) override;

    Result<void> renameAssetOrFolder(const std::filesystem::path& old_path,
                                     const std::filesystem::path& new_path) override;

    uint64_t submitLoadAsset(const AssetLoadRequest& request) override;

    uint64_t submitImportScene(const SceneImportRequest& request) override;

    void reloadAsset(const Guid& guid) override;

    // @TODO: get rid of this
    EngineServices& services();

    // @TODO: deprecate
    [[nodiscard]] std::string resolvePath(const std::filesystem::path& path) override;

    // @TODO: deprecate
    AssetRef loadAssetSync(const Guid& guid) override;

protected:
    AssetRef loadAssetSyncHelper(const Guid& guid);

    uint32_t counter_{ 0 };
    std::mutex asset_lock_;
};

}  // namespace cave
