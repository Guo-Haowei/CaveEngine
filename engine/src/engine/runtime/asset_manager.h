#pragma once
#include "engine/assets/asset_interface.h"
#include "engine/assets/guid.h"
#include "engine/core/base/concurrent_queue.h"
#include "engine/core/base/singleton.h"
#include "engine/runtime/module.h"

namespace cave {

enum class AssetType : uint32_t;
class AssetEntry;
class Scene;

class AssetManager : public Singleton<AssetManager>, public Module {
public:
    struct LoadTask;

    AssetManager()
        : Module("AssetManager") {}

    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    Result<Guid> CreateAsset(AssetType p_type, const std::filesystem::path& p_folder, const char* p_name = nullptr);
    Result<Guid> CreateAsset(AssetType p_type, const std::string& p_short_path);

    auto MoveAsset(const std::filesystem::path& p_old, const std::filesystem::path& p_new) -> Result<void>;

    std::string ResolvePath(const std::filesystem::path& p_path);

    static void WorkerMain();
    static void RequestShutdown();

private:
    AssetRef LoadAssetSync(const Guid& p_guid);

    bool LoadAssetAsync(const Guid& p_guid,
                        AssetLoadSuccessCallback&& p_on_success,
                        AssetLoadFailureCallback&& p_on_failure,
                        void* p_userdata);

    void EnqueueLoadTask(LoadTask& p_task);

    uint32_t m_counter{ 0 };
    std::mutex m_assetLock;
    std::filesystem::path m_assets_root;

    friend class AssetRegistry;
};

}  // namespace cave
