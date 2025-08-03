#include "asset_inspector.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/assets/image_asset.h"
#include "engine/debugger/profiler.h"
#include "engine/runtime/asset_registry.h"

#include "editor/editor_asset_manager.h"
#include "editor/editor_layer.h"
#include "editor/utility/content_entry.h"
#include "editor/viewer/viewer.h"
#include "editor/viewer/viewer_tab.h"
#include "editor/widgets/tool_bar.h"
#include "editor/widgets/drag_drop.h"
#include "editor/widgets/widget.h"

namespace cave {

AssetInspector::AssetInspector(EditorLayer& p_editor)
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

void AssetInspector::UpdateInternal() {
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

    auto& asset_manager = static_cast<EditorAssetManager&>(IAssetManager::GetSingleton());
    const auto& root = asset_manager.GetAssetRoot();
    const int max = static_cast<int>(m_current_path.size()) - 1;
    const ContentEntry* current = Navigate(root.get(), 0, max);
    if (!current) {
        m_current_path = { "@res://" };
        current = root.get();
    }
    DEV_ASSERT(current->is_dir);

    ImVec2 thumbnail_size{ 196, 196 };

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

        auto [hovered, clicked] = DrawAssetCard(image->gpu_texture ? image->gpu_texture->GetHandle() : 0,
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
