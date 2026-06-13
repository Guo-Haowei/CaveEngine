#include "ContentBrowser.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/diagnostics/Log.h"
#include "cave/core/diagnostics/Profiler.h"

#include "editor/EditorAssetManager.h"
#include "editor/EditorState.h"
#include "editor/services/IconCache.h"
#include "editor/services/ThumbnailService.h"
#include "editor/utility/ContentEntry.h"
#include "editor/widgets/DragDrop.h"
#include "editor/widgets/Image.h"
#include "editor/widgets/ToolBar.h"

// @TODO: refactor
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/ui/layout.h"

namespace cave {

ContentBrowser::ContentBrowser(EditorState& editor)
    : EditorWindow(editor) {
    current_path_ = { "@res://" };
}

const char* ContentBrowser::windowId() const {
    return ICON_FA_FOLDER_CLOSED "  Content Browser";
}

void ContentBrowser::onAttach() {
    IconCache& icons = editor_services_.iconCache();
    folder_iamge_ = icons.GetIconHandle(IconName::Folder);
    fallback_iamge_ = icons.GetIconHandle(IconName::Meta);
    thumbnail_lut_[".scene"] = icons.GetIconHandle(IconName::Scene);
    thumbnail_lut_[".sprite_anim"] = icons.GetIconHandle(IconName::Anim);
    thumbnail_lut_[".lua"] = icons.GetIconHandle(IconName::Lua);
    thumbnail_lut_[".tilemap"] = icons.GetIconHandle(IconName::TileMap);
    thumbnail_lut_[".tileset"] = icons.GetIconHandle(IconName::TileSet);

    DEV_ASSERT(folder_iamge_ && fallback_iamge_);
}

void ContentBrowser::drawUIImpl() {
    CAVE_PROFILE_EVENT();
    drawContentBrowser();
}

void ContentBrowser::drawBreadcrumb() {
    int clicked = -1;

    const int len = static_cast<int>(current_path_.size());
    for (int i = 0; i < len; ++i) {
        if (i != 0) {
            ImGui::SameLine(0.0f, 4.0f);
        }

        if (ImGui::Button(current_path_[i].c_str())) {
            clicked = i;
        }
    }
    if (clicked != -1) {
        current_path_.resize(clicked + 1);
    }
}

const ContentEntry* ContentBrowser::navigate(const ContentEntry* p_node,
                                             int p_cur,
                                             int p_max) {
    if (!p_node) {
        return nullptr;
    }

    DEV_ASSERT(p_cur <= p_max);

    const auto& current = current_path_[p_cur];
    if (current != p_node->file_name) {
        return nullptr;
    }

    if (p_cur == p_max) {
        return p_node;
    }

    for (const auto& child : p_node->children) {
        const ContentEntry* match = navigate(child.get(), p_cur + 1, p_max);
        if (match) {
            return match;
        }
    }

    return nullptr;
}

void ContentBrowser::drawContentBrowser() {
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

    drawBreadcrumb();

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
    const int max = static_cast<int>(current_path_.size()) - 1;
    const ContentEntry* current = navigate(root.get(), 0, max);
    if (!current) {
        current_path_ = { "@res://" };
        current = root.get();
    }
    DEV_ASSERT(current->is_dir);

    constexpr uint32_t thumbnail_size = 256;

    ThumbnailService& thumbnail = editor_services_.thumbnail();
    auto find_texture = [&](ContentEntry& p_entry) -> uint64_t {
        if (p_entry.is_dir) return folder_iamge_;
        const AssetMetaData* meta = p_entry.handle.GetMeta();
        if (meta) {
            if (meta->type == AssetType::Image) {
                const ImageAsset* img = p_entry.handle.Get<ImageAsset>();
                if (img && img->gpu_texture) {
                    return img->gpu_texture->GetHandle();
                }
            }

            ThumbnailKey key{
                .guid = p_entry.handle.GetGuid(),
                .size = thumbnail_size,
            };
            if (uint64_t handle = thumbnail.GetOrRequest(key)) {
                return handle;
            }
        }

        if (auto it = thumbnail_lut_.find(p_entry.extension); it != thumbnail_lut_.end()) return it->second;
        return fallback_iamge_;
    };

    for (const auto& node : current->children) {
        const uint64_t handle = find_texture(*node);

        auto [hovered, clicked] = ui::AssetCard(handle,
                                                node->file_name.data(),
                                                math::Vector2f(thumbnail_size));
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
                current_path_.push_back(std::string(node->file_name));
            }
        } else {
            if (hovered) {
                ShowAssetToolTip(thumbnail, *node);
            }
        }

        ImGui::TableNextColumn();
    }

    ImGui::EndTable();
}

}  // namespace cave
