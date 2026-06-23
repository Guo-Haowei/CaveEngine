#pragma once
#include "engine/private/runtime/assets/AssetManager.h"

namespace cave {

struct ContentEntry;
struct ImageAsset;
class FileWatcher;

class EditorAssetManager : public AssetManager {
public:
    EditorAssetManager();
    virtual ~EditorAssetManager();

    Result<void> InitializeImpl() override;
    void FinalizeImpl() override;

    void update() override;

    std::shared_ptr<ImageAsset> findImage(const std::string& name);

    const auto& assetRoot() const { return asset_root_; }
    const auto& folderLut() const { return folder_lut_; }

protected:
    Result<void> addAlwaysLoadImages();
    void rebuildAssetFolderTree();

    std::unordered_map<std::string, std::shared_ptr<ImageAsset>> images_;
    std::unique_ptr<FileWatcher> file_watcher_;

    std::filesystem::path resource_folder_;
    std::unique_ptr<ContentEntry> asset_root_;
    std::unordered_map<std::string, const ContentEntry*> folder_lut_;
};

}  // namespace cave
