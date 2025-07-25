#include "file_system_panel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "engine/core/os/platform_io.h"
#include "engine/core/string/string_utils.h"
#include "engine/runtime/asset_manager.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/common_dvars.h"
#include "editor/editor_layer.h"
#include "editor/widgets/widget.h"

namespace cave {

namespace fs = std::filesystem;

struct FolderTreeNode {
    AssetType type;
    bool is_dir;
    std::filesystem::path sys_path;
    std::string virtual_path;
    std::string_view file_name;
    std::string_view extension;

    FolderTreeNode* parent;
    std::vector<std::unique_ptr<FolderTreeNode>> children;
};

std::unique_ptr<FolderTreeNode> BuildFolderTree(const fs::path& p_sys_path,
                                                FolderTreeNode* p_parent) {
    try {
        if (!fs::exists(p_sys_path)) {
            return nullptr;
        }

        const bool is_dir = fs::is_directory(p_sys_path);
        const bool is_file = fs::is_regular_file(p_sys_path);
        if (!is_dir && !is_file) {
            return nullptr;
        }

        auto node = std::make_unique<FolderTreeNode>();
        node->type = AssetType::Unknown;
        node->extension = "";
        node->is_dir = is_dir;
        node->sys_path = p_sys_path;
        node->parent = p_parent;
        if (p_parent) {
            node->virtual_path = AssetManager::GetSingleton().ResolvePath(p_sys_path);
            node->file_name = StringUtils::FileName(node->virtual_path, '/');
        } else {
            node->virtual_path = "@res://";
            node->file_name = node->virtual_path;
        }

        if (is_file) {
            auto handle = AssetRegistry::GetSingleton().FindByPath(node->virtual_path);
            if (handle.is_none()) {
                return nullptr;
            }

            const AssetMetaData* meta = handle.unwrap_unchecked().GetMeta();
            DEV_ASSERT(meta);
            node->type = meta->type;
            node->extension = StringUtils::Extension(node->file_name);
        } else {
            for (const auto& entry : fs::directory_iterator(p_sys_path)) {
                auto child = BuildFolderTree(entry.path(), node.get());
                if (child) {
                    node->children.push_back(std::move(child));
                }
            }
        }

        return node;
    } catch (const fs::filesystem_error& e) {
        LOG_ERROR("Filesystem error: {}", e.what());
        return nullptr;
    }
}

void FileSystemPanel::DrawFolderTreeNode(const FolderTreeNode& p_node) {
    const bool is_dir = p_node.is_dir;

    int flags = 0;
    flags |= ImGuiTreeNodeFlags_DefaultOpen;
    flags |= !is_dir ? ImGuiTreeNodeFlags_Leaf : 0;

    auto id = std::format("##{}", p_node.virtual_path);

    const bool node_open = ImGui::TreeNodeEx(id.c_str(), flags);

    const std::string& short_path = p_node.virtual_path;

    if (ImGui::BeginPopupContextItem()) {
        FolderPopup(p_node);
        ImGui::EndPopup();
    }

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
        const char* icon = is_dir ? ICON_FA_FOLDER : ICON_FA_CUBE;
        const bool is_file = !p_node.is_dir;
        ImGui::Text("%s %s", icon, p_node.file_name.data());
        if (is_file) {
            // @TODO: refactor
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
            auto asset_registry = m_editor.GetApplication()->GetAssetRegistry();
            auto _handle = asset_registry->FindByPath(short_path);
            if (_handle.is_some()) {
                auto handle = std::move(_handle.unwrap_unchecked());
                IAsset* asset = handle.Get();
                if (DEV_VERIFY(asset)) {
                    if (is_file) {
                        ShowAssetToolTip(*handle.GetMeta(), asset);
                    }
                }
            }
        }
    }

    if (node_open) {
        for (const auto& node : p_node.children) {
            DrawFolderTreeNode(*node);
        }

        ImGui::TreePop();
    }
}

FileSystemPanel::FileSystemPanel(EditorLayer& p_editor)
    : EditorWindow(p_editor) {
}

void FileSystemPanel::OnAttach() {
    const auto& path = m_editor.GetApplication()->GetResourceFolder();
    m_root = fs::path{ path };
}

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
            if (ImGui::MenuItem("Scene")) {
                asset_manager->CreateAsset(AssetType::Scene, p_node.virtual_path);
            }
            if (ImGui::MenuItem("TileSet")) {
                asset_manager->CreateAsset(AssetType::TileSet, p_node.virtual_path);
            }
            if (ImGui::MenuItem("SpriteAnimation")) {
                asset_manager->CreateAsset(AssetType::SpriteAnimation, p_node.virtual_path);
            }
            if (ImGui::MenuItem("TileMap")) {
                asset_manager->CreateAsset(AssetType::TileMap, p_node.virtual_path);
            }
            if (ImGui::MenuItem("Material")) {
                asset_manager->CreateAsset(AssetType::Material, p_node.virtual_path);
            }
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
    auto root = BuildFolderTree(m_root, nullptr);

    DrawFolderTreeNode(*root);
}

}  // namespace cave
