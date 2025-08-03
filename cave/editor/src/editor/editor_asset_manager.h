#pragma once
#include "engine/assets/asset_manager.h"

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

    void Update() override;

    std::shared_ptr<ImageAsset> FindImage(const std::string& p_name);

    const auto& GetAssetRoot() const { return m_asset_root; }
    const auto& GetFolderLut() const { return m_folder_lut; }

protected:
    Result<void> AddAlwaysLoadImages();
    void RebuildAssetFolderTree();

    std::unordered_map<std::string, std::shared_ptr<ImageAsset>> m_images;
    std::unique_ptr<FileWatcher> m_file_watcher;

    std::unique_ptr<ContentEntry> m_asset_root;
    std::unordered_map<std::string, const ContentEntry*> m_folder_lut;
};

}  // namespace cave
