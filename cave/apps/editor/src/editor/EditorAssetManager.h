#pragma once
#include "cave/core/containers/StringHash.h"
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
    ~EditorAssetManager() override;

    Result<void> InitializeImpl() override;
    void FinalizeImpl() override;

    void update() override;

    Ref<ImageAsset> findImage(std::string_view name);

    void onAssetSaved(const AssetChangedEvent& event);

    const auto& assetRoot() const { return m_asset_root; }
    const auto& folderLut() const { return m_folder_lut; }

    EditorServices& editorServices() { return *m_editor_services; }
    void setEditorServices(EditorServices* services) { m_editor_services = services; }

protected:
    Result<void> addAlwaysLoadImages();
    void refreshAssetFolderTree();

    EditorServices* m_editor_services{};

    StringHashMap<Ref<ImageAsset>> m_images;
    Owner<FileWatcher> m_file_watcher;

    std::filesystem::path m_resource_folder;
    Owner<ContentEntry> m_asset_root;
    StringHashMap<const ContentEntry*> m_folder_lut;
};

}  // namespace cave
