#include "folder_tree.h"

#include "engine/assets/image_asset.h"
#include "engine/core/string/string_utils.h"
#include "engine/runtime/asset_manager_interface.h"
#include "engine/runtime/asset_registry.h"

namespace cave {

namespace fs = std::filesystem;

std::unique_ptr<FolderTreeNode> BuildFolderTree(const fs::path& p_sys_path,
                                                FolderTreeNode* p_parent) {
    try {
        if (!fs::exists(p_sys_path)) {
            return nullptr;
        }

        const bool is_dir = fs::is_directory(p_sys_path);
        const bool is_file = fs::is_regular_file(p_sys_path);
        if (!is_dir && !is_file) {
            return nullptr;
        }

        auto node = std::make_unique<FolderTreeNode>();
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
            if (node->type == AssetType::Image) {
                node->thumbnail = node->handle;
            } else {
                std::string thumbnail_path = std::format("@res://_cache/{}@256x256.png", meta->guid.ToString());
                if (auto _handle = AssetRegistry::GetSingleton().FindByPath<ImageAsset>(thumbnail_path); _handle.is_some()) {
                    node->thumbnail = _handle.unwrap_unchecked();
                }
            }
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

void ShowAssetToolTip(const AssetMetaData& p_meta, const IAsset* p_asset) {
    if (ImGui::BeginTooltip()) {
        ImGui::Text("name: %s", p_meta.name.c_str());
        ImGui::Text("import_path: %s", p_meta.import_path.c_str());
        ImGui::Text("type: %s", EnumTraits<AssetType>::ToString(p_meta.type).data());

        if (p_asset) {
            DEV_ASSERT(p_asset->GetType() == AssetType::Image);
            auto texture = reinterpret_cast<const ImageAsset&>(*p_asset);
            const int w = texture.width;
            const int h = texture.height;

            if (texture.gpu_texture) {
                float adjusted_w = std::min(256.f, static_cast<float>(w));
                float adjusted_h = adjusted_w / w * h;
                ImGui::Image(texture.gpu_texture->GetHandle(), ImVec2(adjusted_w, adjusted_h));
            }
        }

        ImGui::EndTooltip();
    }
}

void ShowAssetToolTip(const FolderTreeNode& p_node) {
    if (p_node.is_dir) {
        return;
    }

    const AssetMetaData* meta = p_node.handle.GetMeta();
    if (meta) {
        ShowAssetToolTip(*meta, p_node.thumbnail.Get());
    }
}

}  // namespace cave
