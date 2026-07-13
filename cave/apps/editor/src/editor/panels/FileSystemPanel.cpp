#include "FileSystemPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "cave/core/diagnostics/Profiler.h"

#include "editor/EditorAssetManager.h"
#include "editor/EditorState.h"
#include "editor/utility/ContentEntry.h"
#include "editor/widgets/DragDrop.h"
#include "engine/private/ui/inputs.h"

// @TODO: remove private include
#include "engine/private/core/os/platform_io.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/CommonDvars.h"
#include "engine/private/runtime/framework/VFS.h"

namespace cave {

namespace fs = std::filesystem;

FileSystemPanel::FileSystemPanel(EditorState& editor)
    : EditorWindow(editor) {
}

const char* FileSystemPanel::windowId() const {
    return ICON_FA_FOLDER_CLOSED "  File System";
}

void FileSystemPanel::onAttach() {
    m_root = m_engine_services.vfs().GetMount("@res");
}

void FileSystemPanel::drawFolderTreeNode(const ContentEntry& entry, bool open) {
    const bool is_dir = entry.is_dir;

    int flags = open ? ImGuiTreeNodeFlags_DefaultOpen : 0;
    flags |= !is_dir ? ImGuiTreeNodeFlags_Leaf : 0;

    auto id = std::format("##{}", entry.virtual_path);

    const bool node_open = ImGui::TreeNodeEx(id.c_str(), flags);
    const char* icon = ICON_FA_CUBE;
    if (is_dir) {
        icon = node_open ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER_CLOSED;
    }

    ImGui::SameLine();

    if (m_renaming == entry.sys_path) {
        std::string buffer;
        ImGui::Text("%s", icon);
        ImGui::SameLine();
        if (ui::TextBox(nullptr, buffer)) {
            fs::path to_path = m_renaming.parent_path();
            to_path = to_path / buffer.c_str();
            m_engine_services.assetManager().renameAssetOrFolder(m_renaming, to_path);
            m_renaming = "";
        }
        if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0)) {
            m_renaming = "";
        }
    } else {
        auto text = std::format("{} {}", icon, entry.file_name);
        ImGui::Selectable(text.c_str());
        const bool hovered = ImGui::IsItemHovered();

        if (ImGui::BeginPopupContextItem()) {
            ShowPopup(entry, m_editor_services.document(), [&]() {
                m_renaming = entry.sys_path;
            });
            ImGui::EndPopup();
        }

        DragDropSource_ContentEntry(entry);

        auto& asset_manager = static_cast<EditorAssetManager&>(IAssetManager::singleton());

        DragDropTarget_Folder(entry, asset_manager.folderLut());

        if (hovered) {
            ShowAssetToolTip(m_editor_services.thumbnail(), entry);
        }
    }

    if (node_open) {
        for (const auto& sub_folder : entry.children) {
            drawFolderTreeNode(*sub_folder);
        }

        ImGui::TreePop();
    }
}

void FileSystemPanel::drawUIImpl() {
    CAVE_PROFILE_EVENT();

    auto& asset_manager = static_cast<EditorAssetManager&>(IAssetManager::singleton());

    drawFolderTreeNode(*asset_manager.assetRoot(), true);
}

}  // namespace cave
