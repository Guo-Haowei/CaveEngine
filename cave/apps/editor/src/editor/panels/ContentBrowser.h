#pragma once
#include "engine/private/assets/asset_handle.h"
#include "engine/private/assets/asset_interface.h"
#include "editor/panels/EditorWindow.h"

namespace cave {

struct ContentEntry;
struct ImageAsset;

class ContentBrowser : public EditorWindow {
public:
    ContentBrowser(EditorState& p_editor);

    void OnAttach() override;

    const char* GetWindowId() const override {
        return "Content Browser";
    }

    void DrawContentBrowser();

protected:
    void DrawUIImpl() override;

    const ContentEntry* Navigate(const ContentEntry* p_node, int p_cur, int p_max);
    void DrawBreadcrumb();

    std::vector<std::string> m_current_path;

    std::shared_ptr<ImageAsset> m_folder_iamge;
    std::shared_ptr<ImageAsset> m_fallback_iamge;
    std::unordered_map<std::string_view, std::shared_ptr<ImageAsset>> m_thumbnail_lut;
};

}  // namespace cave
