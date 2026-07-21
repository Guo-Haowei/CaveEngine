#include "ContentEntry.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "cave/core/string/StringUtils.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

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

// @TODO:
static constexpr int kThumbnailSize = 256;

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

        auto node = MakeOwner<ContentEntry>();
        node->asset_type = AssetType::Unknown;
        node->extension = "";
        node->is_dir = is_dir;
        node->sys_path = sys_path;
        node->parent = parent;
        if (parent) {
            node->virtual_path = IAssetManager::singleton().resolvePath(sys_path);
            node->file_name = StringUtils::fileName(node->virtual_path, '/');
        } else {
            node->virtual_path = "@res://";
            node->file_name = node->virtual_path;
        }

        if (node->file_name == "_cache") {
            // ignore _cache folder
            return nullptr;
        }

        if (is_file) {
            auto handle = AssetRegistry::singleton().findByPath(node->virtual_path);
            if (handle.is_none()) {
                return nullptr;
            }
            node->handle = handle.unwrap_unchecked();
            const AssetMetaData* meta = node->handle.meta();

            DEV_ASSERT(meta);
            node->asset_type = meta->type;
            node->extension = StringUtils::extension(node->file_name);
#if 0
            if (node->type == AssetType::Image) {
                node->thumbnail = node->handle;
            } else {
                std::string thumbnail_path = std::format("@res://_cache/{}@256x256.png", meta->guid.ToString());
                if (auto _handle = AssetRegistry::singleton().findByPath<ImageAsset>(thumbnail_path)) {
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

void ShowAssetToolTip(ThumbnailService& thumbnail, const AssetHandle& handle) {
    const AssetMetaData* meta = handle.meta();
    DEV_ASSERT(meta);

    if (ImGui::BeginTooltip()) {
        ImGui::Text("name: %s", meta->name.c_str());
        ImGui::Text("import_path: %s", meta->import_path.c_str());
        ImGui::Text("type: %s", EnumTraits<AssetType>::ToString(meta->type).data());

        switch (meta->type) {
            case AssetType::Image: {
                const auto* texture = dynamic_cast<const ImageAsset*>(handle.get());
                if (texture && texture->gpu_texture) {
                    ui::CenteredImage(texture->gpu_texture->GetHandle(),
                                      kThumbnailSize,
                                      texture->width,
                                      texture->height,
                                      false);
                }
            } break;
            case AssetType::TileSet: {
                if (const auto* tile_set = dynamic_cast<const TileSetAsset*>(handle.get())) {
                    const auto* texture = dynamic_cast<const ImageAsset*>(tile_set->handle().get());
                    if (texture && texture->gpu_texture) {
                        ui::CenteredImage(texture->gpu_texture->GetHandle(),
                                          kThumbnailSize,
                                          texture->width,
                                          texture->height,
                                          false);
                    }
                }
            } break;
            case AssetType::Material:
            case AssetType::Mesh:
            case AssetType::Prefab:
            case AssetType::Scene: {
                ThumbnailKey key{
                    .guid = handle.guid(),
                    .size = kThumbnailSize,
                };
                const uint64_t texture = thumbnail.getOrRequest(key);
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

static void ShowFolderPopup(const ContentEntry& node) {
    auto& asset_manager = IAssetManager::singleton();

    if (ImGui::BeginMenu("Add")) {
        if (ImGui::MenuItem("Folder")) {
            fs::create_directory(node.sys_path / "NewFolder");
        }

#define ADD_ASSET_MENU(TYPE)                                                      \
    do {                                                                          \
        if (ImGui::MenuItem(#TYPE)) {                                             \
            auto res = asset_manager.createAsset(AssetType::TYPE, node.sys_path); \
            if (!res) {                                                           \
                LOG_ERROR("Failed to create asset: {}", ToString(res.error()));   \
            }                                                                     \
        }                                                                         \
    } while (0)

        ADD_ASSET_MENU(Scene);
        ADD_ASSET_MENU(Prefab);
        ADD_ASSET_MENU(SpriteAnimation);
        ADD_ASSET_MENU(Material);
        ADD_ASSET_MENU(TileSet);

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

            if (auto path = os::OpenFileDialog(filter)) {
                fs::path dest = node.sys_path;
                IAssetManager::singleton().submitImportScene({ path.unwrap_unchecked(), dest });
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Delete")) {
        fs::remove_all(node.sys_path);
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
            desc.guid = node.handle.guid();
            desc.asset_type = node.handle.meta()->type;
            document.openDoc(desc);
        }
        if (ImGui::MenuItem("Save")) {
            const Guid guid = node.handle.guid();
            document.save(guid);
        }
    }

    if (ImGui::MenuItem("Reveal In File Explorer")) {
        cave::os::RevealInFolder(node.sys_path);
    }
}

const char* GetContentIcon(const ContentEntry& entry, bool is_open) {
    if (entry.is_dir) {
        return is_open ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER_CLOSED;
    }

    switch (entry.asset_type) {
        case AssetType::Scene:
            return ICON_FA_MAP;
        case AssetType::Image:
            return ICON_FA_FILE_IMAGE;
        case AssetType::SpriteAnimation:
            return ICON_FA_FILE_VIDEO;
        default:
            return ICON_FA_CUBE;
    }
}

}  // namespace cave
