#include "ContentBrowser.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/private/assets/image_asset.h"
#include "engine/private/core/debugger/Profiler.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

#include "editor/EditorAssetManager.h"
#include "editor/EditorState.h"
#include "editor/services/IconCache.h"
#include "editor/utility/ContentEntry.h"
#include "editor/widgets/DragDrop.h"
#include "editor/widgets/Image.h"
#include "editor/widgets/ToolBar.h"
#include "engine/private/ui/layout.h"
#include "engine/private/assets/asset_handle.h"
#include "engine/private/assets/asset_interface.h"

namespace cave {

ContentBrowser::ContentBrowser(EditorState& p_editor)
    : EditorWindow(p_editor) {
    m_current_path = { "@res://" };
}

void ContentBrowser::OnAttach() {
    IconCache& icons = m_editor.IconCache();
    m_folder_iamge = icons.GetIconHandle(IconName::Folder);
    m_fallback_iamge = icons.GetIconHandle(IconName::Meta);
    m_thumbnail_lut[".scene"] = icons.GetIconHandle(IconName::Scene);
    m_thumbnail_lut[".sprite_anim"] = icons.GetIconHandle(IconName::Anim);
    m_thumbnail_lut[".lua"] = icons.GetIconHandle(IconName::Lua);
    m_thumbnail_lut[".tilemap"] = icons.GetIconHandle(IconName::TileMap);
    m_thumbnail_lut[".tileset"] = icons.GetIconHandle(IconName::TileSet);

    DEV_ASSERT(m_folder_iamge && m_fallback_iamge);
}

void ContentBrowser::DrawUIImpl() {
    CAVE_PROFILE_EVENT();
    DrawContentBrowser();
}

void ContentBrowser::DrawBreadcrumb() {
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

const ContentEntry* ContentBrowser::Navigate(const ContentEntry* p_node,
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
        const ContentEntry* match = Navigate(child.get(), p_cur + 1, p_max);
        if (match) {
            return match;
        }
    }

    return nullptr;
}

void ContentBrowser::DrawContentBrowser() {
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

    // @TODO: reuse this part
    ImVec2 window_size = ImGui::GetContentRegionAvail();
    constexpr float desired_icon_size = 280.f;
    int num_col = static_cast<int>(glm::floor(window_size.x / desired_icon_size));
    num_col = glm::max(1, num_col);

    ImGui::BeginTable("Inner", num_col);
    ImGui::TableNextColumn();

    auto& asset_manager = static_cast<EditorAssetManager&>(IAssetManager::GetSingleton());
    const auto& root = asset_manager.GetAssetRoot();
    const int max = static_cast<int>(m_current_path.size()) - 1;
    const ContentEntry* current = Navigate(root.get(), 0, max);
    if (!current) {
        m_current_path = { "@res://" };
        current = root.get();
    }
    DEV_ASSERT(current->is_dir);

    math::Vector2f thumbnail_size(256);

    for (const auto& node : current->children) {
        uint64_t handle = 0;
        if (node->is_dir) {
            handle = m_folder_iamge;
        } else {
            if (ImageAsset* image = node->thumbnail.Get()) {
                handle = image->gpu_texture->GetHandle();
            } else {
                auto it = m_thumbnail_lut.find(node->extension);
                if (it == m_thumbnail_lut.end()) {
                    handle = m_fallback_iamge;
                } else {
                    handle = it->second;
                }
            }
        }

        auto [hovered, clicked] = ui::AssetCard(handle, node->file_name.data(), thumbnail_size);
        if (ImGui::BeginPopupContextItem()) {
            ShowPopup(*node, m_editor, []() {
                LOG_WARN("TODO: rename");
            });
            ImGui::EndPopup();
        }

        DragDropSourceContentEntry(*node);

        DragDropTargetFolder(*node, asset_manager.GetFolderLut());

        if (node->is_dir) {
            if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                m_current_path.push_back(std::string(node->file_name));
            }
        } else {
            if (hovered) {
                ShowAssetToolTip(*node);
            }
        }

        ImGui::TableNextColumn();
    }

    ImGui::EndTable();
}

}  // namespace cave
