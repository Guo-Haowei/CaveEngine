#include "FileSystemPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "cave/core/diagnostics/Profiler.h"
#include "engine/private/core/os/platform_io.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/CommonDvars.h"
#include "engine/private/runtime/framework/VFS.h"

#include "editor/EditorAssetManager.h"
#include "editor/EditorState.h"
#include "editor/utility/ContentEntry.h"
#include "editor/widgets/DragDrop.h"
#include "engine/private/ui/inputs.h"

namespace cave {

namespace fs = std::filesystem;

FileSystemPanel::FileSystemPanel(EditorState& p_editor)
    : EditorWindow(p_editor) {
}

void FileSystemPanel::OnAttach() {
    m_root = m_editor.app().GetVFS().GetMount("@res");
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
        if (ui::TextBox(nullptr, buffer.data(), (uint32_t)buffer.size())) {
            fs::path to_path = m_renaming.parent_path();
            to_path = to_path / buffer.c_str();
            if (is_dir) {
                fs::rename(m_renaming, to_path);
            } else {
                m_editor.app().GetAssetManager()->MoveAsset(m_renaming, to_path);
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
            ShowPopup(p_node, m_editor, [&]() {
                m_renaming = p_node.sys_path;
            });
            ImGui::EndPopup();
        }

        DragDropSourceContentEntry(p_node);

        auto& asset_manager = static_cast<EditorAssetManager&>(IAssetManager::GetSingleton());

        DragDropTargetFolder(p_node, asset_manager.GetFolderLut());

        if (hovered) {
            ShowAssetToolTip(m_editor.ThumbnailService(), p_node);
        }
    }

    if (node_open) {
        for (const auto& node : p_node.children) {
            DrawFolderTreeNode(*node);
        }

        ImGui::TreePop();
    }
}

void FileSystemPanel::DrawUIImpl() {
    CAVE_PROFILE_EVENT();

    auto& asset_manager = static_cast<EditorAssetManager&>(IAssetManager::GetSingleton());

    DrawFolderTreeNode(*asset_manager.GetAssetRoot());
}

}  // namespace cave
