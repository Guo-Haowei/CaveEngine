#include "ContentEntry.h"

#include "engine/private/core/os/platform_io.h"
#include "engine/private/core/string/StringUtils.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

#include "editor/services/DocumentService.h"
#include "editor/services/EditService.h"
#include "editor/services/ThumbnailService.h"
#include "editor/EditorState.h"

namespace cave {

namespace fs = std::filesystem;

std::unique_ptr<ContentEntry> BuildFolderTree(const fs::path& p_sys_path,
                                              ContentEntry* p_parent) {
    try {
        if (!fs::exists(p_sys_path)) {
            return nullptr;
        }

        const bool is_dir = fs::is_directory(p_sys_path);
        const bool is_file = fs::is_regular_file(p_sys_path);
        if (!is_dir && !is_file) {
            return nullptr;
        }

        auto node = std::make_unique<ContentEntry>();
        node->type = AssetType::Unknown;
        node->extension = "";
        node->is_dir = is_dir;
        node->sys_path = p_sys_path;
        node->parent = p_parent;
        if (p_parent) {
            node->virtual_path = IAssetManager::GetSingleton().ResolvePath(p_sys_path);
            node->file_name = StringUtils::FileName(node->virtual_path, '/');
        } else {
            node->virtual_path = "@res://";
            node->file_name = node->virtual_path;
        }

        if (node->file_name == "_cache") {
            // ignore _cache folder
            return nullptr;
        }

        if (is_file) {
            auto handle = AssetRegistry::GetSingleton().FindByPath(node->virtual_path);
            if (handle.is_none()) {
                return nullptr;
            }
            node->handle = handle.unwrap_unchecked();
            const AssetMetaData* meta = node->handle.GetMeta();

            DEV_ASSERT(meta);
            node->type = meta->type;
            node->extension = StringUtils::Extension(node->file_name);
#if 0
            if (node->type == AssetType::Image) {
                node->thumbnail = node->handle;
            } else {
                std::string thumbnail_path = std::format("@res://_cache/{}@256x256.png", meta->guid.ToString());
                if (auto _handle = AssetRegistry::GetSingleton().FindByPath<ImageAsset>(thumbnail_path); _handle.is_some()) {
                    node->thumbnail = _handle.unwrap_unchecked();
                }
            }
#endif
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

// @TODO: 
static constexpr int kThumbnailSize = 256;

void ShowAssetToolTip(ThumbnailService& p_service, const AssetHandle& p_handle) {
    const AssetMetaData* meta = p_handle.GetMeta();
    DEV_ASSERT(meta);

    if (ImGui::BeginTooltip()) {
        ImGui::Text("name: %s", meta->name.c_str());
        ImGui::Text("import_path: %s", meta->import_path.c_str());
        ImGui::Text("type: %s", EnumTraits<AssetType>::ToString(meta->type).data());

        switch (meta->type) {
            case AssetType::Image: {
                auto texture = reinterpret_cast<const ImageAsset&>(*p_handle.Get());
                if (texture.gpu_texture) {
                    const int w = texture.width;
                    const int h = texture.height;
                    const float adjusted_w = (float)std::min(kThumbnailSize, w);
                    const float adjusted_h = adjusted_w / w * h;
                    ImGui::Image(texture.gpu_texture->GetHandle(), ImVec2(adjusted_w, adjusted_h));
                }
            } break;
            case AssetType::Material:
            case AssetType::Mesh:
            case AssetType::Scene: {
                ThumbnailKey key{
                    .guid = p_handle.GetGuid(),
                    .size = kThumbnailSize,
                };
                const uint64_t texture = p_service.GetOrRequest(key);
                if (texture) {
                    ImGui::Image(texture, ImVec2(kThumbnailSize, kThumbnailSize));
                }
            } break;
            default:
                break;
        }

        ImGui::EndTooltip();
    }
}

void ShowAssetToolTip(ThumbnailService& p_service, const ContentEntry& p_node) {
    if (p_node.is_dir) return;
    ShowAssetToolTip(p_service, p_node.handle);
}

static void ShowFolderPopup(const ContentEntry& p_node) {
    auto& asset_manager = IAssetManager::GetSingleton();

    if (ImGui::BeginMenu("Add")) {
        if (ImGui::MenuItem("Folder")) {
            fs::create_directory(p_node.sys_path / "NewFolder");
        }

#define ADD_ASSET_MENU(TYPE)                                                        \
    do {                                                                            \
        if (ImGui::MenuItem(#TYPE)) {                                               \
            auto res = asset_manager.CreateAsset(AssetType::TYPE, p_node.sys_path); \
            if (!res) {                                                             \
                LOG_ERROR("Failed to create asset: {}", ToString(res.error()));     \
            }                                                                       \
        }                                                                           \
    } while (0)

        ADD_ASSET_MENU(Scene);
        ADD_ASSET_MENU(SpriteAnimation);
        ADD_ASSET_MENU(Material);
        ADD_ASSET_MENU(TileSet);
        ADD_ASSET_MENU(TileMap);

        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Import")) {
        if (ImGui::MenuItem("Scene")) {
            std::vector<const char*> filter = {
                ".gltf",
                ".glb",
                ".obj",
                ".fbx",
            };

            if (auto path = os::OpenFileDialog(filter); path.is_some()) {
                fs::path dest = p_node.sys_path;
                IAssetManager::GetSingleton().SubmitImportScene({ path.unwrap_unchecked(), dest });
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Delete")) {
        fs::remove_all(p_node.sys_path);
    }
}

void ShowPopup(const ContentEntry& p_node,
               EditorState& p_editor,
               std::function<void(void)> p_rename_cb) {
    if (ImGui::MenuItem("Rename")) {
        if (p_rename_cb) {
            p_rename_cb();
        }
    }

    if (p_node.is_dir) {
        ShowFolderPopup(p_node);
    } else {
        if (ImGui::MenuItem("Edit")) {
            OpenDocDesc desc;
            desc.guid = p_node.handle.GetGuid();
            desc.asset_type = p_node.handle.GetMeta()->type;
            p_editor.DocumentService().OpenDoc(desc);
        }
        if (ImGui::MenuItem("Save")) {
            AssetRegistry::GetSingleton().SaveAsset(p_node.handle.GetGuid());
        }
    }

    if (ImGui::MenuItem("Reveal In File Explorer")) {
        cave::os::RevealInFolder(p_node.sys_path);
    }
}

}  // namespace cave
