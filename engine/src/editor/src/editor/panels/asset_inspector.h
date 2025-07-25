#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/assets/asset_interface.h"
#include "editor/editor_window.h"

namespace cave {

class AssetRegistry;
struct FolderTreeNode;

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

    const FolderTreeNode* Navigate(const FolderTreeNode* p_node);

    std::string m_current_path;
};

}  // namespace cave
