#pragma once
#include "engine/assets/asset_interface.h"
#include "engine/core/base/concurrent_queue.h"
#include "engine/core/base/singleton.h"
#include "engine/runtime/module.h"

namespace cave {

enum class AssetType : uint32_t;
class AssetEntry;
class Scene;

class IAssetManager : public Singleton<IAssetManager>,
                      public Module,
                      public ModuleCreateRegistry<IAssetManager> {
public:
    IAssetManager()
        : Module("AssetManager") {}

    virtual Result<Guid> CreateAsset(AssetType p_type, const std::filesystem::path& p_folder, const char* p_name = nullptr) = 0;
    virtual Result<Guid> CreateAsset(AssetType p_type, const std::string& p_short_path) = 0;

    virtual Result<void> MoveAsset(const std::filesystem::path& p_old, const std::filesystem::path& p_new) = 0;

    virtual std::string ResolvePath(const std::filesystem::path& p_path) = 0;

    virtual bool LoadAssetAsync(const Guid& p_guid,
                                AssetLoadSuccessCallback&& p_on_success,
                                AssetLoadFailureCallback&& p_on_failure,
                                void* p_userdata) = 0;

    virtual bool ImportScene(const std::string& p_path) = 0;
};

}  // namespace cave
