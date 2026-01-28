#pragma once
#include "editor/windows/EditorWindow.h"

namespace cave {

struct ContentEntry;

class FileSystemPanel : public EditorWindow {
public:
    FileSystemPanel(EditorState& p_editor);

    void OnAttach() override;

    const char* GetWindowId() const override {
        return "File System";
    }

protected:
    void UpdateInternal(float p_timestep) override;

    void DrawFolderTreeNode(const ContentEntry& p_node);

    std::filesystem::path m_root;
    std::filesystem::path m_renaming;
};

}  // namespace cave
