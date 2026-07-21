#include "ContentBrowser.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/string/StringUtils.h"
#include "cave/core/diagnostics/Profiler.h"

#include "editor/EditorAssetManager.h"
#include "editor/EditorState.h"
#include "editor/services/DragDropService.h"
#include "editor/services/IconCache.h"
#include "editor/services/ThumbnailService.h"
#include "editor/services/Workspace.h"
#include "editor/utility/ContentEntry.h"
#include "editor/widgets/Image.h"
#include "editor/widgets/ToolBar.h"

// @TODO: refactor
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/ui/Layout.h"

namespace cave {

void ContentBrowser::CurrentPath::splitVirtualPath(std::string_view path) {
    constexpr std::string_view kRoot = "@res://";
    m_parts.push_back(std::string(kRoot));
    if (path.empty() || path == kRoot) {
        return;
    }

    DEV_ASSERT(path.starts_with(kRoot));

    std::string s(path.data() + kRoot.size());

    StringSplitter split(s.data());
    while (split.canAdvance()) {
        std::string_view part = split.advance('/');
        m_parts.push_back(std::string(part));
    }
}

std::string ContentBrowser::CurrentPath::joinVirtualPath() const {
    if (m_parts.empty()) {
        return "@res://";
    }

    std::string out = m_parts[0];

    for (size_t i = 1; i < m_parts.size(); ++i) {
        if (!out.ends_with('/')) {
            out += '/';
        }

        out += m_parts[i];
    }

    return out;
}

ContentBrowser::ContentBrowser(EditorState& editor)
    : EditorWindow(editor) {
}

const char* ContentBrowser::windowId() const {
    return ICON_FA_FOLDER_CLOSED "  Content Browser";
}

void ContentBrowser::onAttach() {
    std::string_view current_path = m_editor_services.workspace().workspaceState().content_browser.current_path;
    m_path.splitVirtualPath(current_path);

    m_path.setPropertyChangeCallback([this]() {
        auto& state = m_editor_services.workspace().workspaceState();
        state.content_browser.current_path = m_path.joinVirtualPath();
        state.markDirty();
    });

    IconCache& icons = m_editor_services.iconCache();
    m_folder_iamge = icons.getIconHandle(IconName::Folder);
    m_fallback_iamge = icons.getIconHandle(IconName::Meta);
    m_thumbnail_lut[".scene"] = icons.getIconHandle(IconName::Scene);
    m_thumbnail_lut[".prefab"] = icons.getIconHandle(IconName::Scene);
    m_thumbnail_lut[".sprite_anim"] = icons.getIconHandle(IconName::Anim);
    m_thumbnail_lut[".lua"] = icons.getIconHandle(IconName::Lua);
    m_thumbnail_lut[".tileset"] = icons.getIconHandle(IconName::TileSet);

    DEV_ASSERT(m_folder_iamge && m_fallback_iamge);
}

void ContentBrowser::onDetach() {
}

void ContentBrowser::drawUIImpl() {
    CAVE_PROFILE_EVENT();
    drawContentBrowser();
}

void ContentBrowser::drawBreadcrumb() {
    int clicked = -1;

    const int len = static_cast<int>(m_path.size());
    for (int i = 0; i < len; ++i) {
        if (i != 0) {
            ImGui::SameLine(0.0f, 4.0f);
        }

        if (ImGui::Button(m_path.at(i).c_str())) {
            clicked = i;
        }
    }
    if (clicked != -1) {
        m_path.getMut().resize(clicked + 1);
        m_path.onPropertyChange();
    }
}

const ContentEntry* ContentBrowser::navigate(const ContentEntry* node,
                                             int cur,
                                             int max) {
    if (!node) {
        return nullptr;
    }

    DEV_ASSERT(cur <= max);

    const auto& current = m_path.at(cur);
    if (current != node->file_name) {
        return nullptr;
    }

    if (cur == max) {
        return node;
    }

    for (const auto& child : node->children) {
        if (const ContentEntry* match = navigate(child.get(), cur + 1, max)) {
            return match;
        }
    }

    return nullptr;
}

void ContentBrowser::drawContentBrowser() {
    drawBreadcrumb();

    // @TODO: reuse this part
    ImVec2 window_size = ImGui::GetContentRegionAvail();
    constexpr float desired_icon_size = 280.f;
    int num_col = static_cast<int>(glm::floor(window_size.x / desired_icon_size));
    num_col = glm::max(1, num_col);

    ImGui::BeginTable("Inner", num_col);
    ImGui::TableNextColumn();

    auto& asset_manager = static_cast<EditorAssetManager&>(IAssetManager::singleton());
    const auto& root = asset_manager.assetRoot();
    const int max = static_cast<int>(m_path.size()) - 1;
    const ContentEntry* current = navigate(root.get(), 0, max);
    if (!current) {
        m_path.getMut() = { "@res://" };
        m_path.onPropertyChange();
        current = root.get();
    }
    DEV_ASSERT(current->is_dir);

    constexpr uint32_t thumbnail_size = 256;

    ThumbnailService& thumbnail = m_editor_services.thumbnail();
    auto find_texture = [&](ContentEntry& p_entry) -> uint64_t {
        if (p_entry.is_dir) return m_folder_iamge;
        const AssetMetaData* meta = p_entry.handle.meta();
        if (meta) {
            if (meta->type == AssetType::Image) {
                const ImageAsset* img = p_entry.handle.get<ImageAsset>();
                if (img && img->gpu_texture) {
                    return img->gpu_texture->GetHandle();
                }
            }

            ThumbnailKey key{
                .guid = p_entry.handle.guid(),
                .size = thumbnail_size,
            };
            if (uint64_t handle = thumbnail.getOrRequest(key)) {
                return handle;
            }
        }

        if (auto it = m_thumbnail_lut.find(p_entry.extension); it != m_thumbnail_lut.end()) return it->second;
        return m_fallback_iamge;
    };

    for (const auto& node : current->children) {
        const uint64_t handle = find_texture(*node);

        auto [hovered, clicked] = ui::AssetCard(
            handle,
            node->file_name.data(),
            math::Vec2f(thumbnail_size)
#if 0
            , [this](uint64_t tex, const ImVec2& min, const ImVec2& max) {
                m_engine_services.imgui->drawTexture(*ImGui::GetWindowDrawList(), tex, min, max);
            }
#endif
        );
        if (ImGui::BeginPopupContextItem()) {
            ShowPopup(*node, m_editor_services.document(), []() {
                LOG_WARN("TODO: rename");
            });
            ImGui::EndPopup();
        }

        auto& drag_drop = m_editor_services.dragDrop();
        drag_drop.dragContentEntry(*node);
        drag_drop.dropFolder(*node, asset_manager.folderLut());

        if (node->is_dir) {
            if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                m_path.add(std::string(node->file_name));
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
