#include "file_system_panel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "engine/core/os/platform_io.h"
#include "engine/runtime/asset_manager_interface.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/common_dvars.h"
#include "editor/editor_layer.h"
#include "editor/utility/folder_tree.h"
#include "editor/widgets/widget.h"

namespace cave {

namespace fs = std::filesystem;

// ----------------- @TODO: refactor drag drop --------------------------
using AssetID = uint64_t;
using EntityID = uint32_t;
using PathStr = std::string;  // for virtual FS paths

enum class DragKind : uint32_t {
    Asset, // assign guid
    Folder, // move folder
    Entity,
    Material, // apply to mesh directly
};

enum class DropOp : uint8_t { None,
                              Copy,
                              Move,
                              Link,
                              Create };

struct DragPayload {
    DragKind kind;
    // union-like data (POD): no pointers to UI state
    std::vector<AssetID> assets;
    std::vector<PathStr> folders;
    std::vector<EntityID> entities;
    // optional extra info
    std::string sourcePanel;  // "ContentBrowser", "Hierarchy", "Viewport"
};
// ----------------- @TODO: refactor drag drop --------------------------

FileSystemPanel::FileSystemPanel(EditorLayer& p_editor)
    : EditorWindow(p_editor) {
}

void FileSystemPanel::OnAttach() {
    const auto& path = m_editor.GetApplication()->GetResourceFolder();
    m_root = fs::path{ path };
}

void FileSystemPanel::DrawFolderTreeNode(const FolderTreeNode& p_node) {
    const bool is_dir = p_node.is_dir;

    int flags = 0;
    flags |= ImGuiTreeNodeFlags_DefaultOpen;
    flags |= !is_dir ? ImGuiTreeNodeFlags_Leaf : 0;

    auto id = std::format("##{}", p_node.virtual_path);

    const bool node_open = ImGui::TreeNodeEx(id.c_str(), flags);

    const std::string& short_path = p_node.virtual_path;

    ImGui::SameLine();

    if (m_renaming == p_node.sys_path) {
        std::string buffer;
        buffer.resize(256);
        if (DrawInputText(nullptr, buffer)) {
            fs::path to_path = m_renaming.parent_path();
            to_path = to_path / buffer.c_str();
            m_editor.GetApplication()->GetAssetManager()->MoveAsset(m_renaming, to_path);
            m_renaming = "";
        }
        if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0)) {
            m_renaming = "";
        }
    } else {
        const char* icon = ICON_FA_CUBE;
        if (is_dir) {
            icon = node_open ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER_CLOSED;
        }
        auto text = std::format("{} {}", icon, p_node.file_name);
        ImGui::Selectable(text.c_str());

        if (ImGui::BeginPopupContextItem()) {
            FolderPopup(p_node);
            ImGui::EndPopup();
        }

        const bool is_file = !p_node.is_dir;
        if (is_file) {
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                const char* data = short_path.c_str();
                ImGui::SetDragDropPayload(ASSET_DRAG_DROP_PAYLOAD,
                                          data,
                                          short_path.length() + 1);
                ImGui::Text("Dragging '%s'", data);
                ImGui::EndDragDropSource();
            }
        }

        const bool hovered = ImGui::IsItemHovered();
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

// @TODO: refactor this to work for both content browser and file system
void FileSystemPanel::FolderPopup(const FolderTreeNode& p_node) {
    if (ImGui::MenuItem("Rename")) {
        m_renaming = p_node.sys_path;
    }
    if (ImGui::MenuItem("Reveal In File Explorer")) {
        cave::os::RevealInFolder(p_node.sys_path);
    }
    if (p_node.is_dir) {
        auto asset_manager = m_editor.GetApplication()->GetAssetManager();
        // @TODO: [SCRUM-222] refactor this
        if (ImGui::BeginMenu("Add")) {
#define ADD_ASSET_MENU(TYPE)                                                         \
    do {                                                                             \
        if (ImGui::MenuItem(#TYPE)) {                                                \
            auto res = asset_manager->CreateAsset(AssetType::TYPE, p_node.sys_path); \
            if (!res) {                                                              \
                LOG_ERROR("Failed to create asset: {}", ToString(res.error()));      \
            }                                                                        \
        }                                                                            \
    } while (0)

            ADD_ASSET_MENU(Scene);
            ADD_ASSET_MENU(SpriteAnimation);
            ADD_ASSET_MENU(Material);
            ADD_ASSET_MENU(TileSet);
            ADD_ASSET_MENU(TileMap);

            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Delete")) {
            LOG_ERROR("{}", p_node.virtual_path);
        }
        return;
    }

    auto asset_registry = m_editor.GetApplication()->GetAssetRegistry();
    auto _handle = asset_registry->FindByPath(p_node.virtual_path);
    if (_handle.is_none()) {
        return;
    }

    auto handle = _handle.unwrap_unchecked();

    if (ImGui::MenuItem("Edit")) {
        m_editor.CommandInspectAsset(handle.GetGuid());
    }

    if (ImGui::MenuItem("Save")) {
        asset_registry->SaveAsset(handle.GetGuid());
    }
}

void FileSystemPanel::UpdateInternal() {
    DrawFolderTreeNode(*m_editor.GetAssetRoot());
}

}  // namespace cave
