#pragma once
#include "engine/private/runtime/assets/AssetManager.h"
#include "editor/services/EditorServices.h"

namespace cave {

struct ContentEntry;
struct ImageAsset;
class FileWatcher;

enum class AssetChangeReason : uint8_t {
    Saved,
};

struct AssetChangedEvent {
    AssetChangeReason reason;
    uint64_t revision;
    Guid guid;
};

class EditorAssetManager : public AssetManager {
public:
    EditorAssetManager();
    virtual ~EditorAssetManager();

    Result<void> InitializeImpl() override;
    void FinalizeImpl() override;

    void update() override;

    std::shared_ptr<ImageAsset> findImage(const std::string& name);

    void onAssetSaved(const AssetChangedEvent& event);

    const auto& assetRoot() const { return asset_root_; }
    const auto& folderLut() const { return folder_lut_; }

    EditorServices& editorServices() { return *editor_services_; }
    void setEditorServices(EditorServices* services) { editor_services_ = services; }

protected:
    Result<void> addAlwaysLoadImages();
    void refreshAssetFolderTree();
    void refreshDependencies();

    EditorServices* editor_services_{};

    std::unordered_map<std::string, std::shared_ptr<ImageAsset>> images_;
    std::unique_ptr<FileWatcher> file_watcher_;

    std::filesystem::path resource_folder_;
    std::unique_ptr<ContentEntry> asset_root_;
    std::unordered_map<std::string, const ContentEntry*> folder_lut_;
};

}  // namespace cave
