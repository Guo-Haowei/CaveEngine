#pragma once
#include "editor/panels/EditorWindow.h"

namespace cave {

struct ContentEntry;

class FileSystemPanel : public EditorWindow {
public:
    FileSystemPanel(EditorState& editor);

    void onAttach() override;

    const char* windowId() const override;

protected:
    void drawUIImpl() override;

    void drawFolderTreeNode(const ContentEntry& node, bool open = false);

    std::filesystem::path m_root;
    std::filesystem::path m_renaming;
};

}  // namespace cave
