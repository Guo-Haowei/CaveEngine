#pragma once
#include "editor/panels/EditorWindow.h"

namespace cave {

struct ContentEntry;

class ContentBrowser : public EditorWindow {
public:
    ContentBrowser(EditorState& editor);

    void OnAttach() override;

    const char* windowId() const override;

protected:
    void drawUIImpl() override;

    void drawContentBrowser();
    void drawBreadcrumb();

    const ContentEntry* navigate(const ContentEntry* node, int cur, int p_max);

    std::vector<std::string> current_path_;

    uint64_t folder_iamge_;
    uint64_t fallback_iamge_;
    std::unordered_map<std::string_view, uint64_t> thumbnail_lut_;
};

}  // namespace cave
