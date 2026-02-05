#pragma once
#include "editor/panels/EditorWindow.h"

namespace cave {

struct ContentEntry;

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

    uint64_t m_folder_iamge;
    uint64_t m_fallback_iamge;
    std::unordered_map<std::string_view, uint64_t> m_thumbnail_lut;
};

}  // namespace cave
