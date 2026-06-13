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

    void drawFolderTreeNode(const ContentEntry& node);

    std::filesystem::path root_;
    std::filesystem::path renaming_;
};

}  // namespace cave
