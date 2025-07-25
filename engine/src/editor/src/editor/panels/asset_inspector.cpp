#include "asset_inspector.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/assets/image_asset.h"
#include "engine/runtime/asset_registry.h"

#include "editor/editor_asset_manager.h"
#include "editor/editor_layer.h"
#include "editor/utility/folder_tree.h"
#include "editor/viewer/viewer.h"
#include "editor/viewer/viewer_tab.h"
#include "editor/widgets/tool_bar.h"
#include "editor/widgets/widget.h"

namespace cave {

AssetInspector::AssetInspector(EditorLayer& p_editor)
    : EditorWindow(p_editor) {
    m_current_path = { "@res://" };
}

void AssetInspector::OnAttach() {
    auto& asset_manager = static_cast<EditorAssetManager&>(IAssetManager::GetSingleton());
    m_folder_iamge = asset_manager.FindImage("folder_icon.png");
    m_meta_image = asset_manager.FindImage("meta_icon.png");

    DEV_ASSERT(m_folder_iamge && m_meta_image);
}

void AssetInspector::UpdateInternal() {
    if (ViewerTab* tab = m_editor.GetViewer().GetActiveTab(); tab) {
        tab->DrawAssetInspector();
    } else {
        DrawContentBrowser();
    }
}

void AssetInspector::DrawBreadcrumb() {
    int clicked = -1;

    const int len = static_cast<int>(m_current_path.size());
    for (int i = 0; i < len; ++i) {
        if (i != 0) {
            ImGui::SameLine(0.0f, 4.0f);
        }

        if (ImGui::Button(m_current_path[i].c_str())) {
            clicked = i;
        }
    }
    if (clicked != -1) {
        m_current_path.resize(clicked + 1);
    }
}

const FolderTreeNode* AssetInspector::Navigate(const FolderTreeNode* p_node,
                                               int p_cur,
                                               int p_max) {
    if (!p_node) {
        return nullptr;
    }

    DEV_ASSERT(p_cur <= p_max);

    const auto& current = m_current_path[p_cur];
    if (current != p_node->file_name) {
        return nullptr;
    }

    if (p_cur == p_max) {
        return p_node;
    }

    for (const auto& child : p_node->children) {
        const FolderTreeNode* match = Navigate(child.get(), p_cur + 1, p_max);
        if (match) {
            return match;
        }
    }

    return nullptr;
}

static auto DrawAssetCard(ImTextureID p_texture_id,
                          const char* p_name,
                          ImVec2 p_image_size) -> std::tuple<bool, bool> {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();

    const float rounding = 6.0f;
    const float padding = 6.0f;
    const float spacing = 4.0f;
    const float shadow_offset = 5.0f;

    // Estimate text height: 2 lines + padding
    float text_height = ImGui::GetFontSize() * 2 + spacing * 2;
    ImVec2 card_size = ImVec2(p_image_size.x + padding * 2,
                              p_image_size.y + text_height + 8);

    // Shadow behind card
    draw->AddRectFilled(pos + ImVec2(shadow_offset, shadow_offset),
                        pos + card_size + ImVec2(shadow_offset, shadow_offset),
                        IM_COL32(10, 10, 10, 160),
                        rounding);

    // Card background (lighter than ImGui window)
    ImU32 card_bg = IM_COL32(40, 40, 40, 255);
    ImGui::PushStyleColor(ImGuiCol_Button, card_bg);  // just for convention
    draw->AddRectFilled(pos, pos + card_size, card_bg, rounding);
    ImGui::PopStyleColor();

    ImGui::InvisibleButton(p_name, card_size);
    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();

    // Image (square)
    ImVec2 image_pos = pos + ImVec2(padding, padding);
    draw->AddImage(p_texture_id, image_pos, image_pos + p_image_size);

    // Text
    ImVec2 textStart = image_pos + ImVec2(0, p_image_size.y + spacing);
    draw->AddText(textStart, IM_COL32(180, 180, 180, 220), p_name);

    if (hovered) {
        // draw->AddRect(pos, pos + card_size, IM_COL32(255, 255, 255, 100), rounding, 0, 1.5f);
    }

    return { hovered, clicked };
}

void AssetInspector::DrawContentBrowser() {
    std::vector<ToolBarButtonDesc> descs = {
        { ICON_FA_FOLDER_CLOSED, "Placeholder",
          []() {
          } },
        { ICON_FA_FOLDER_OPEN, "Placeholder",
          []() {
          } },
        { ICON_FA_FOLDER_TREE, "Placeholder",
          []() {
          } },
    };

    std::vector<const ToolBarButtonDesc*> d;
    for (const auto& it : descs) {
        d.push_back(&it);
    }

    DrawToolBar(d);

    DrawBreadcrumb();

    // thumbnails

    ImVec2 window_size = ImGui::GetContentRegionAvail();
    constexpr float desired_icon_size = 224.f;
    int num_col = static_cast<int>(glm::floor(window_size.x / desired_icon_size));
    num_col = glm::max(1, num_col);

    ImGui::BeginTable("Inner", num_col);
    ImGui::TableNextColumn();

    const auto& root = m_editor.GetAssetRoot();
    const int max = static_cast<int>(m_current_path.size()) - 1;
    const FolderTreeNode* current = Navigate(root.get(), 0, max);
    if (!current) {
        m_current_path = { "@res://" };
        current = root.get();
    }
    DEV_ASSERT(current->is_dir);

    ImVec2 thumbnail_size{ 196, 196 };

    for (const auto& node : current->children) {
        ImageAsset* image = node->is_dir ? m_folder_iamge.get() : m_meta_image.get();

        auto [hovered, clicked] = DrawAssetCard(image->gpu_texture ? image->gpu_texture->GetHandle() : 0,
                                                node->file_name.data(),
                                                thumbnail_size);

        if (node->is_dir) {
            if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                m_current_path.push_back(std::string(node->file_name));
            }
        } else {
            if (hovered) {
                const IAsset* asset = node->handle.Get();
                const AssetMetaData* meta = node->handle.GetMeta();
                if (asset && meta) {
                    ShowAssetToolTip(*meta, asset);
                }
            }
        }

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
