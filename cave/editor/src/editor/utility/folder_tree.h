#pragma once
#include "engine/assets/asset_handle.h"

namespace cave {

struct FolderTreeNode {
    AssetType type;
    AssetHandle handle;

    bool is_dir;
    std::filesystem::path sys_path;
    std::string virtual_path;
    std::string_view file_name;
    std::string_view extension;

    FolderTreeNode* parent;
    std::vector<std::unique_ptr<FolderTreeNode>> children;
};

std::unique_ptr<FolderTreeNode> BuildFolderTree(const std::filesystem::path& p_sys_path,
                                                FolderTreeNode* p_parent);

}  // namespace cave
