#include "file_system_panel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "engine/core/os/platform_io.h"
#include "engine/runtime/asset_manager_interface.h"
#include "engine/runtime/common_dvars.h"

#include "editor/editor_layer.h"
#include "editor/utility/content_entry.h"
#include "editor/widgets/drag_drop.h"
#include "editor/widgets/widget.h"

namespace cave {

namespace fs = std::filesystem;

FileSystemPanel::FileSystemPanel(EditorLayer& p_editor)
    : EditorWindow(p_editor) {
}

void FileSystemPanel::OnAttach() {
    const auto& path = m_editor.GetApplication()->GetResourceFolder();
    m_root = fs::path{ path };
}

void FileSystemPanel::DrawFolderTreeNode(const ContentEntry& p_node) {
    const bool is_dir = p_node.is_dir;

    int flags = 0;
    flags |= ImGuiTreeNodeFlags_DefaultOpen;
    flags |= !is_dir ? ImGuiTreeNodeFlags_Leaf : 0;

    auto id = std::format("##{}", p_node.virtual_path);

    const bool node_open = ImGui::TreeNodeEx(id.c_str(), flags);
    const char* icon = ICON_FA_CUBE;
    if (is_dir) {
        icon = node_open ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER_CLOSED;
    }

    ImGui::SameLine();

    if (m_renaming == p_node.sys_path) {
        std::string buffer;
        buffer.resize(256);
        ImGui::Text("%s", icon);
        ImGui::SameLine();
        if (DrawInputText(nullptr, buffer)) {
            fs::path to_path = m_renaming.parent_path();
            to_path = to_path / buffer.c_str();
            if (is_dir) {
                fs::rename(m_renaming, to_path);
            } else {
                m_editor.GetApplication()->GetAssetManager()->MoveAsset(m_renaming, to_path);
            }
            m_renaming = "";
        }
        if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0)) {
            m_renaming = "";
        }
    } else {
        auto text = std::format("{} {}", icon, p_node.file_name);
        ImGui::Selectable(text.c_str());
        const bool hovered = ImGui::IsItemHovered();

        if (ImGui::BeginPopupContextItem()) {
            ShowPopup(p_node, m_editor, &m_renaming);
            ImGui::EndPopup();
        }

        DragDropSourceContentEntry(p_node);

        DragDropTargetFolder(p_node, m_editor.GetFolderLut());

        if (hovered) {
            ShowAssetToolTip(p_node);
        }
    }

    if (node_open) {
        for (const auto& node : p_node.children) {
            DrawFolderTreeNode(*node);
        }

        ImGui::TreePop();
    }
}

void FileSystemPanel::UpdateInternal() {
    DrawFolderTreeNode(*m_editor.GetAssetRoot());
}

}  // namespace cave
