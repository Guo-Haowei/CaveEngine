#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/assets/asset_interface.h"
#include "editor/editor_window.h"

namespace cave {

class AssetRegistry;
struct ContentEntry;
struct ImageAsset;

class AssetInspector : public EditorWindow {
public:
    AssetInspector(EditorLayer& p_editor);

    void OnAttach() override;

    const char* GetTitle() const override {
        return "Asset Inspector";
    }

    void DrawContentBrowser();

protected:
    void UpdateInternal() override;

    const ContentEntry* Navigate(const ContentEntry* p_node, int p_cur, int p_max);
    void DrawBreadcrumb();

    std::vector<std::string> m_current_path;

    std::shared_ptr<ImageAsset> m_folder_iamge;
    std::shared_ptr<ImageAsset> m_fallback_iamge;
    std::unordered_map<std::string_view, std::shared_ptr<ImageAsset>> m_thumbnail_lut;
};

}  // namespace cave
