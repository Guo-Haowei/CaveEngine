#pragma once
#include "cave/core/base/Singleton.h"

#include "cave/runtime/assets/IAsset.h"
#include "cave/runtime/framework/IService.h"

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
                      public IService,
                      public ServiceCreateRegistry<IAssetManager> {
public:
    IAssetManager()
        : IService("AssetManager") {}

    virtual void update() = 0;

    virtual Result<Guid> createAsset(AssetType type, const std::filesystem::path& folder, const char* name = nullptr) = 0;
    virtual Result<Guid> createAsset(AssetType type, const std::string& short_path) = 0;

    virtual Result<void> renameAssetOrFolder(const std::filesystem::path& old_path, const std::filesystem::path& new_path) = 0;

    virtual uint64_t submitLoadAsset(const AssetLoadRequest& request) = 0;

    virtual uint64_t submitImportScene(const SceneImportRequest& request) = 0;

    virtual void reloadAsset(const Guid& guid) = 0;

    // @TODO: deprecate
    virtual std::string resolvePath(const std::filesystem::path& path) = 0;

    // @TODO: deprecate
    virtual AssetRef loadAssetSync(const Guid& guid) = 0;

    // @TODO: remove this
    virtual Ref<ImageAsset> findImage(std::string_view) { return nullptr; }
};

}  // namespace cave
