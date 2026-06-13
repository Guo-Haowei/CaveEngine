#include "ContentEntry.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/core/string/StringUtils.h"

#include "engine/private/core/os/platform_io.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

#include "editor/services/DocumentService.h"
#include "editor/services/EditService.h"
#include "editor/services/ThumbnailService.h"
#include "editor/widgets/Image.h"

namespace cave {

namespace fs = std::filesystem;

auto BuildFolderTree(const fs::path& sys_path,
                     ContentEntry* parent) -> std::unique_ptr<ContentEntry> {
    try {
        if (!fs::exists(sys_path)) {
            return nullptr;
        }

        const bool is_dir = fs::is_directory(sys_path);
        const bool is_file = fs::is_regular_file(sys_path);
        if (!is_dir && !is_file) {
            return nullptr;
        }

        auto node = std::make_unique<ContentEntry>();
        node->type = AssetType::Unknown;
        node->extension = "";
        node->is_dir = is_dir;
        node->sys_path = sys_path;
        node->parent = parent;
        if (parent) {
            node->virtual_path = IAssetManager::GetSingleton().ResolvePath(sys_path);
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
            for (const auto& entry : fs::directory_iterator(sys_path)) {
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

void ShowAssetToolTip(ThumbnailService& thumbnail, const AssetHandle& handle) {
    const AssetMetaData* meta = handle.GetMeta();
    DEV_ASSERT(meta);

    if (ImGui::BeginTooltip()) {
        ImGui::Text("name: %s", meta->name.c_str());
        ImGui::Text("import_path: %s", meta->import_path.c_str());
        ImGui::Text("type: %s", EnumTraits<AssetType>::ToString(meta->type).data());

        switch (meta->type) {
            case AssetType::Image: {
                auto texture = reinterpret_cast<const ImageAsset&>(*handle.Get());
                if (texture.gpu_texture) {
                    ui::CenteredImage(texture.gpu_texture->GetHandle(),
                                      kThumbnailSize,
                                      texture.width,
                                      texture.height,
                                      false);
                }
            } break;
            case AssetType::Material:
            case AssetType::Mesh:
            case AssetType::Scene: {
                ThumbnailKey key{
                    .guid = handle.GetGuid(),
                    .size = kThumbnailSize,
                };
                const uint64_t texture = thumbnail.GetOrRequest(key);
                ui::CenteredImage(texture, kThumbnailSize, kThumbnailSize, kThumbnailSize, false);
            } break;
            default:
                break;
        }

        ImGui::EndTooltip();
    }
}

void ShowAssetToolTip(ThumbnailService& thumbnail, const ContentEntry& node) {
    if (node.is_dir) return;
    ShowAssetToolTip(thumbnail, node.handle);
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

void ShowPopup(const ContentEntry& node,
               DocumentService& document,
               std::function<void(void)> rename_cb) {
    if (ImGui::MenuItem("Rename")) {
        if (rename_cb) {
            rename_cb();
        }
    }

    if (node.is_dir) {
        ShowFolderPopup(node);
    } else {
        if (ImGui::MenuItem("Edit")) {
            OpenDocDesc desc;
            desc.guid = node.handle.GetGuid();
            desc.asset_type = node.handle.GetMeta()->type;
            document.openDoc(desc);
        }
        if (ImGui::MenuItem("Save")) {
            const Guid guid = node.handle.GetGuid();
            document.save(guid);
        }
    }

    if (ImGui::MenuItem("Reveal In File Explorer")) {
        cave::os::RevealInFolder(node.sys_path);
    }
}

}  // namespace cave
