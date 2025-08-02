#pragma once
#include "editor/editor_window.h"

namespace cave {

struct ContentEntry;

class FileSystemPanel : public EditorWindow {
public:
    FileSystemPanel(EditorLayer& p_editor);

    void OnAttach() override;

    const char* GetTitle() const override {
        return "File System";
    }

protected:
    void UpdateInternal() override;

    void DrawFolderTreeNode(const ContentEntry& p_node);

    std::filesystem::path m_root;
    std::filesystem::path m_renaming;
};

}  // namespace cave
