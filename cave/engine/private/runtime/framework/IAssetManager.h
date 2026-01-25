#pragma once
#include "cave/runtime/core/Singleton.h"

#include "engine/private/assets/asset_interface.h"
#include "engine/private/core/base/concurrent_queue.h"
#include "engine/private/runtime/framework/Module.h"

namespace cave {

enum class AssetType : uint32_t;
class AssetEntry;
class Scene;

// @TODO: remove
struct ImageAsset;

struct AssetLoadRequest {
    Guid guid;
};

struct SceneImportRequest {
    std::filesystem::path source_path;
    std::filesystem::path dest_dir;
};

class IAssetManager : public Singleton<IAssetManager>,
                      public Module,
                      public ModuleCreateRegistry<IAssetManager> {
public:
    IAssetManager()
        : Module("AssetManager") {}

    virtual void Update() = 0;

    virtual Result<Guid> CreateAsset(AssetType p_type, const std::filesystem::path& p_folder, const char* p_name = nullptr) = 0;
    virtual Result<Guid> CreateAsset(AssetType p_type, const std::string& p_short_path) = 0;

    virtual Result<void> MoveAsset(const std::filesystem::path& p_old, const std::filesystem::path& p_new) = 0;

    virtual uint64_t SubmitLoadAsset(const AssetLoadRequest& p_request) = 0;

    virtual uint64_t SubmitImportScene(const SceneImportRequest& p_request) = 0;

    // @TODO: deprecate
    virtual std::string ResolvePath(const std::filesystem::path& p_path) = 0;

    // @TODO: deprecate
    virtual AssetRef LoadAssetSync(const Guid& p_guid) = 0;

    // @TODO: remove this
    virtual std::shared_ptr<ImageAsset> FindImage(const std::string&) { return nullptr; }
};

}  // namespace cave
