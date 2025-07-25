#include "asset_inspector.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/assets/image_asset.h"
#include "engine/runtime/asset_registry.h"

#include "editor/editor_layer.h"
#include "editor/utility/folder_tree.h"
#include "editor/viewer/viewer.h"
#include "editor/viewer/viewer_tab.h"
#include "editor/widgets/tool_bar.h"

namespace cave {

AssetInspector::AssetInspector(EditorLayer& p_editor)
    : EditorWindow(p_editor) {
    m_current_path = "@res://";
}

void AssetInspector::OnAttach() {
}

void AssetInspector::UpdateInternal() {
    if (ViewerTab* tab = m_editor.GetViewer().GetActiveTab(); tab) {
        tab->DrawAssetInspector();
    } else {
        DrawContentBrowser();
    }
}

static void DrawBreadcrumb(const std::vector<std::string>& pathParts, std::function<void(int)> onClick) {
    for (size_t i = 0; i < pathParts.size(); ++i) {
        // Clickable button for each segment
        if (ImGui::Button(pathParts[i].c_str())) {
            onClick(static_cast<int>(i));  // user-defined callback
        }

        // Draw separator " > " if not last
        if (i + 1 < pathParts.size()) {
            ImGui::SameLine(0.0f, 4.0f);  // small spacing
            ImGui::TextUnformatted(">");
            ImGui::SameLine(0.0f, 4.0f);
        }
    }
}

const FolderTreeNode* AssetInspector::Navigate(const FolderTreeNode* p_node) {
    if (!p_node) {
        return nullptr;
    }

    if (p_node->virtual_path == m_current_path) {
        return p_node;
    }

    if (m_current_path.starts_with(p_node->virtual_path)) {
        for (const auto& child : p_node->children) {
            const FolderTreeNode* match = Navigate(child.get());
            if (match) {
                return match;
            }
        }
    }

    return nullptr;
}

void AssetInspector::DrawContentBrowser() {
    std::vector<ToolBarButtonDesc> descs = {
        { ICON_FA_FOLDER, "Placeholder",
          []() {
          } },
        { ICON_FA_FOLDER_CLOSED, "Placeholder",
          []() {
          } },
        { ICON_FA_FOLDER_MINUS, "Placeholder",
          []() {
          } },
        { ICON_FA_FOLDER_OPEN, "Placeholder",
          []() {
          } },
        { ICON_FA_FOLDER_PLUS, "Placeholder",
          []() {
          } },
        { ICON_FA_FOLDER_TREE, "Placeholder",
          []() {
          } },
        { ICON_FA_BACKWARD, "Placeholder",
          []() {
          } },
    };

    std::vector<const ToolBarButtonDesc*> d;
    for (const auto& it : descs) {
        d.push_back(&it);
    }

    DrawToolBar(d);

    std::vector<std::string> paths = { "Assets", "Textures", "Brick" };
    DrawBreadcrumb(paths, [](int) {
        // Handle breadcrumb click — truncate to that level, navigate, etc.
        // std::cout << "Clicked breadcrumb at level " << level << std::endl;
    });

    ImVec2 window_size = ImGui::GetContentRegionAvail();
    constexpr float desired_icon_size = 150.f;
    int num_col = static_cast<int>(glm::floor(window_size.x / desired_icon_size));
    num_col = glm::max(1, num_col);

    ImGui::BeginTable("Inner", num_col);
    ImGui::TableNextColumn();

    AssetRegistry& registry = AssetRegistry::GetSingleton();
    ImVec2 thumbnail_size{ 120, 120 };

    auto handle = registry.FindByPath<ImageAsset>("@persist://textures/checkerboard").unwrap();
    ImageAsset* image = handle.Get();

    const auto& root = m_editor.GetAssetRoot();
    const FolderTreeNode* current = Navigate(root.get());
    current = current ? current : root.get();
    DEV_ASSERT(current->is_dir);

    for (const auto& node : current->children) {
        const auto& path = node->file_name;

        bool clicked = false;
        if (image->gpu_texture) {
            clicked = ImGui::ImageButton(path.data(),
                                         (ImTextureID)image->gpu_texture->GetHandle(),
                                         thumbnail_size);
        } else {
            clicked = ImGui::Button(path.data(), thumbnail_size);
        }

        ImGui::Text("%s", path.data());
        ImGui::TableNextColumn();
    }

    ImGui::EndTable();
}

}  // namespace cave

#if 0
void ContentBrowser::OnAttach() {
    const auto& path = m_editor.GetApplication()->GetResourceFolder();
    m_rootPath = fs::path{ path };
    m_currentPath = m_rootPath;

    auto asset_registry = m_editor.GetApplication()->GetAssetRegistry();
    auto folder_icon = asset_registry->GetAssetByHandle<ImageAsset>(AssetHandle{ "@res://images/icons/folder_icon.png" });
    auto image_icon = asset_registry->GetAssetByHandle<ImageAsset>(AssetHandle{ "@res://images/icons/image_icon.png" });
    auto scene_icon = asset_registry->GetAssetByHandle<ImageAsset>(AssetHandle{ "@res://images/icons/scene_icon.png" });
    auto meta_icon = asset_registry->GetAssetByHandle<ImageAsset>(AssetHandle{ "@res://images/icons/meta_icon.png" });

    m_iconMap["."] = { folder_icon, nullptr };
    m_iconMap[".png"] = { image_icon, nullptr };
    m_iconMap[".meta"] = { meta_icon, nullptr };
    m_iconMap[".hdr"] = { image_icon, EditorItem::DRAG_DROP_ENV };
    m_iconMap[".gltf"] = { scene_icon, EditorItem::DRAG_DROP_IMPORT };
    m_iconMap[".obj"] = { scene_icon, EditorItem::DRAG_DROP_IMPORT };
    m_iconMap[".scene"] = { scene_icon, EditorItem::DRAG_DROP_IMPORT };
    m_iconMap[".lua"] = { scene_icon, EditorItem::DRAG_DROP_IMPORT };
}

void ContentBrowser::DrawSideBarHelper(const std::filesystem::path& p_path) {
    if (!fs::exists(p_path)) {
        return;
    }

    for (const auto& entry : fs::directory_iterator(p_path)) {
        const bool is_file = entry.is_regular_file();
        const bool is_dir = entry.is_directory();
        if (!is_dir && !is_file) {
            continue;
        }

        fs::path full_path = entry.path();
        auto stem = full_path.stem().string();

        std::string name;
        if (is_dir) {
            name = ICON_FA_FOLDER " ";
            name.append(stem);
        } else {
            name = ICON_FA_FILE " ";
            name = full_path.filename().string();
        }

        if (ImGui::TreeNode(name.c_str())) {
            if (is_dir) {
                DrawSideBarHelper(full_path);
            }
            ImGui::TreePop();
        }
    }
}

void ContentBrowser::DrawSideBar() {
    for (const auto& entry : fs::directory_iterator(m_rootPath)) {
        const bool is_dir = entry.is_directory();
        if (!is_dir) {
            continue;
        }
        fs::path full_path = entry.path();
        auto stem = full_path.stem().string();
        bool should_skip = true;
        AssetType type = AssetType::NONE;
        if (stem == "images") {
            should_skip = false;
            type = AssetType::IMAGE;
        }
        if (stem == "scripts") {
            should_skip = false;
            type = AssetType::TEXT;
        }
        if (should_skip) {
            continue;
        }

        std::string name;
        if (is_dir) {
            name = ICON_FA_FOLDER " ";
            name.append(stem);
        } else {
            name = ICON_FA_FILE " ";
            name = full_path.filename().string();
        }

        if (ImGui::TreeNode(name.c_str())) {
            m_assetType = type;
            if (is_dir) {
                DrawSideBarHelper(full_path);
            }
            ImGui::TreePop();
        }
    }
}

void ContentBrowser::UpdateInternal(Scene&) {
}


void ContentBrowser::DrawDetailPanel() {
#if 0
    if (ImGui::Button("<-")) {
        if (m_currentPath != m_rootPath) {
            m_currentPath = m_currentPath.parent_path();
        }
    }

    // ImGui::Image((ImTextureID)handle, ImVec2(dim.x, dim.y), ImVec2(0, 1), ImVec2(1, 0));

    ImVec2 window_size = ImGui::GetWindowSize();
    constexpr float desired_icon_size = 120.f;
    int num_col = static_cast<int>(glm::floor(window_size.x / desired_icon_size));
    num_col = glm::max(1, num_col);

    ImGui::Columns(num_col, nullptr, false);

    for (const auto& entry : fs::directory_iterator(m_currentPath)) {
        const bool is_file = entry.is_regular_file();
        const bool is_dir = entry.is_directory();
        if (!is_dir && !is_file) {
            continue;
        }

        fs::path full_path = entry.path();
        fs::path relative_path = fs::relative(full_path, m_rootPath);
        // std::string relative_path_string = relative_path.string();

        std::string name;
        std::string extention;
        if (is_dir) {
            name = full_path.stem().string();
            extention = ".";
        } else if (is_file) {
            name = full_path.filename().string();
            extention = full_path.extension().string();
        }

        auto it = m_iconMap.find(extention);
        ImVec2 size{ desired_icon_size, desired_icon_size };
        if (it == m_iconMap.end()) {
            continue;
        }

        bool clicked = false;
        bool has_texture = false;

        if (it->second.image) {
            auto texture = it->second.image->gpu_texture;
            if (texture) {
                clicked = ImGui::ImageButton(name.c_str(), (ImTextureID)texture->GetHandle(), size);
                has_texture = true;
            }
        }
        if (!has_texture) {
            clicked = ImGui::Button(name.c_str(), size);
        }

        if (is_file) {
            std::string full_path_string = full_path.string();
            char* dragged_data = StringUtils::Strdup(full_path_string.c_str());
            const char* action = it->second.action;

            if (action) {
                ImGuiDragDropFlags flags = ImGuiDragDropFlags_SourceNoDisableHover;
                if (ImGui::BeginDragDropSource(flags)) {
                    ImGui::SetDragDropPayload(action, &dragged_data, sizeof(const char*));
                    ImGui::Text("%s", name.c_str());
                    ImGui::EndDragDropSource();
                }
            }
        }

        ImGui::Text("%s", name.c_str());

        if (clicked) {
            if (is_dir) {
                m_currentPath = full_path;
            } else if (is_file) {
            }
        }

        ImGui::NextColumn();
    }

    ImGui::Columns(1);
#endif
}

}  // namespace my

#endif
