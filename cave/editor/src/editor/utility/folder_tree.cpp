#include "folder_tree.h"

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

}  // namespace cave
