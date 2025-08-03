#pragma once
#include "engine/assets/asset_manager.h"

namespace cave {

class FileWatcher;
struct ImageAsset;

class EditorAssetManager : public AssetManager {
public:
    Result<void> InitializeImpl() override;

    std::shared_ptr<ImageAsset> FindImage(const std::string& p_name);

protected:
    Result<void> AddAlwaysLoadImages();

    std::unordered_map<std::string, std::shared_ptr<ImageAsset>> m_images;
    std::unique_ptr<FileWatcher> m_file_watcher;
};

}  // namespace cave
