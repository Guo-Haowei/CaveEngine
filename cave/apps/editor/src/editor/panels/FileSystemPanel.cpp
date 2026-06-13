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

FileSystemPanel::FileSystemPanel(EditorState& editor)
    : EditorWindow(editor) {
}

const char* FileSystemPanel::windowId() const {
    return ICON_FA_FOLDER_CLOSED "  File System";
}

void FileSystemPanel::onAttach() {
    root_ = app_services_.vfs().GetMount("@res");
}

void FileSystemPanel::drawFolderTreeNode(const ContentEntry& entry) {
    const bool is_dir = entry.is_dir;

    int flags = 0;
    flags |= ImGuiTreeNodeFlags_DefaultOpen;
    flags |= !is_dir ? ImGuiTreeNodeFlags_Leaf : 0;

    auto id = std::format("##{}", entry.virtual_path);

    const bool node_open = ImGui::TreeNodeEx(id.c_str(), flags);
    const char* icon = ICON_FA_CUBE;
    if (is_dir) {
        icon = node_open ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER_CLOSED;
    }

    ImGui::SameLine();

    if (renaming_ == entry.sys_path) {
        std::string buffer;
        buffer.resize(256);
        ImGui::Text("%s", icon);
        ImGui::SameLine();
        if (ui::TextBox(nullptr, buffer.data(), (uint32_t)buffer.size())) {
            fs::path to_path = renaming_.parent_path();
            to_path = to_path / buffer.c_str();
            if (is_dir) {
                fs::rename(renaming_, to_path);
            } else {
                app_services_.assetManager().MoveAsset(renaming_, to_path);
            }
            renaming_ = "";
        }
        if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0)) {
            renaming_ = "";
        }
    } else {
        auto text = std::format("{} {}", icon, entry.file_name);
        ImGui::Selectable(text.c_str());
        const bool hovered = ImGui::IsItemHovered();

        if (ImGui::BeginPopupContextItem()) {
            ShowPopup(entry, m_editor, [&]() {
                renaming_ = entry.sys_path;
            });
            ImGui::EndPopup();
        }

        DragDropSourceContentEntry(entry);

        auto& asset_manager = static_cast<EditorAssetManager&>(IAssetManager::GetSingleton());

        DragDropTargetFolder(entry, asset_manager.GetFolderLut());

        if (hovered) {
            ShowAssetToolTip(editor_services_.thumbnail(), entry);
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

    auto& asset_manager = static_cast<EditorAssetManager&>(IAssetManager::GetSingleton());

    drawFolderTreeNode(*asset_manager.GetAssetRoot());
}

}  // namespace cave
