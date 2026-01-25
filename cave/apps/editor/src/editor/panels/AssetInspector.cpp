#include "AssetInspector.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/assets/image_asset.h"
#include "engine/debugger/profiler.h"
#include "engine/runtime/framework/AssetRegistry.h"

#include "editor/EditorAssetManager.h"
#include "editor/EditorState.h"
#include "editor/utility/ContentEntry.h"
#include "editor/viewer/Viewer.h"
#include "editor/viewer/ViewerTab.h"
#include "editor/widgets/DragDrop.h"
#include "editor/widgets/Image.h"
#include "editor/widgets/ToolBar.h"
#include "engine/ui/layout.h"

namespace cave {

AssetInspector::AssetInspector(EditorState& p_editor)
    : EditorWindow(p_editor) {
    m_current_path = { "@res://" };
}

void AssetInspector::OnAttach() {
    auto& asset_manager = static_cast<EditorAssetManager&>(IAssetManager::GetSingleton());
    m_folder_iamge = asset_manager.FindImage("folder_icon.png");
    m_fallback_iamge = asset_manager.FindImage("meta_icon.png");
    m_thumbnail_lut[".scene"] = asset_manager.FindImage("scene@256x256.png");
    m_thumbnail_lut[".sprite_anim"] = asset_manager.FindImage("anim@256x256.png");
    m_thumbnail_lut[".lua"] = asset_manager.FindImage("script@256x256.png");
    m_thumbnail_lut[".tilemap"] = asset_manager.FindImage("tileset@256x256.png");
    m_thumbnail_lut[".tileset"] = asset_manager.FindImage("tileset@256x256.png");

    DEV_ASSERT(m_folder_iamge && m_fallback_iamge);
}

void AssetInspector::UpdateInternal(float) {
    CAVE_PROFILE_EVENT();
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

const ContentEntry* AssetInspector::Navigate(const ContentEntry* p_node,
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

    // @TODO: reuse this part
    ImVec2 window_size = ImGui::GetContentRegionAvail();
    constexpr float desired_icon_size = 224.f;
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

    Vector2f thumbnail_size(196);

    for (const auto& node : current->children) {
        ImageAsset* image = nullptr;
        if (node->is_dir) {
            image = m_folder_iamge.get();
        } else {
            if (!(image = node->thumbnail.Get())) {
                auto it = m_thumbnail_lut.find(node->extension);
                if (it == m_thumbnail_lut.end()) {
                    image = m_fallback_iamge.get();
                } else {
                    image = it->second.get();
                }
            }
        }

        auto [hovered, clicked] = ui::AssetCard(image->gpu_texture ? image->gpu_texture->GetHandle() : 0,
                                                node->file_name.data(),
                                                thumbnail_size);
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
