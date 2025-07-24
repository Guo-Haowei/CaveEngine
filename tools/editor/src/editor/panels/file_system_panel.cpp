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

FileSystemPanel::FileSystemPanel(EditorLayer& p_editor)
    : EditorWindow(p_editor) {
}

void FileSystemPanel::OnAttach() {
    const auto& path = m_editor.GetApplication()->GetResourceFolder();
    m_root = fs::path{ path };
}

void FileSystemPanel::FolderPopup(const std::filesystem::path& p_path, const std::string& p_short_path, bool p_is_dir) {
    if (ImGui::MenuItem("Rename")) {
        m_renaming = p_path;
    }
    if (ImGui::MenuItem("Reveal In File Explorer")) {
        cave::os::RevealInFolder(p_path);
    }
    if (p_is_dir) {
        auto asset_manager = m_editor.GetApplication()->GetAssetManager();
        // @TODO: [SCRUM-222] refactor this
        if (ImGui::BeginMenu("Add")) {
            if (ImGui::MenuItem("Scene")) {
                asset_manager->CreateAsset(AssetType::Scene, p_path);
            }
            if (ImGui::MenuItem("TileSet")) {
                asset_manager->CreateAsset(AssetType::TileSet, p_path);
            }
            if (ImGui::MenuItem("SpriteAnimation")) {
                asset_manager->CreateAsset(AssetType::SpriteAnimation, p_path);
            }
            if (ImGui::MenuItem("TileMap")) {
                asset_manager->CreateAsset(AssetType::TileMap, p_path);
            }
            if (ImGui::MenuItem("Material")) {
                asset_manager->CreateAsset(AssetType::Material, p_path);
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Delete")) {
            LOG_ERROR("{}", p_path.string());
        }
        return;
    }

    auto asset_registry = m_editor.GetApplication()->GetAssetRegistry();
    auto _handle = asset_registry->FindByPath(p_short_path);
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

void FileSystemPanel::ListFile(const std::filesystem::path& p_path, const char* p_name_override) {
    if (!fs::exists(p_path)) {
        return;
    }

    const bool is_dir = fs::is_directory(p_path);
    const bool is_file = fs::is_regular_file(p_path);

    std::string filename = p_path.filename().string();
    auto ext = StringUtils::Extension(filename);
    if (ext == ".meta") {
        return;
    }

    const char* icon = is_dir ? ICON_FA_FOLDER : ICON_FA_CUBE;

    int flags = 0;
    flags |= ImGuiTreeNodeFlags_DefaultOpen;
    flags |= is_file ? ImGuiTreeNodeFlags_Leaf : 0;

    auto id = std::format("{} ##{}", icon, p_path.string());

    const bool node_open = ImGui::TreeNodeEx(id.c_str(), flags);

    std::string short_path = m_editor.GetApplication()->GetAssetManager()->ResolvePath(p_path);

    if (ImGui::BeginPopupContextItem()) {
        FolderPopup(p_path, short_path, is_dir);
        ImGui::EndPopup();
    }

    ImGui::SameLine();

    if (m_renaming == p_path) {
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
        ImGui::Text("%s", p_name_override ? p_name_override : filename.c_str());
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

    if (node_open && is_dir) {
        for (const auto& entry : fs::directory_iterator(p_path)) {
            ListFile(entry.path());
        }
    }
    if (node_open) {
        ImGui::TreePop();
    }
}

void FileSystemPanel::UpdateInternal() {
    ListFile(m_root, "@res://");
}

}  // namespace cave
