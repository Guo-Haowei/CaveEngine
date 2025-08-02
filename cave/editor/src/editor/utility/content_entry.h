#pragma once
#include "engine/assets/asset_handle.h"

namespace cave {

class EditorLayer;

struct ContentEntry {
    AssetType type;
    AssetHandle handle;
    Handle<ImageAsset> thumbnail;

    bool is_dir;
    std::filesystem::path sys_path;
    std::string virtual_path;
    std::string_view file_name;
    std::string_view extension;

    ContentEntry* parent;
    std::vector<std::unique_ptr<ContentEntry>> children;
};

std::unique_ptr<ContentEntry> BuildFolderTree(const std::filesystem::path& p_sys_path,
                                              ContentEntry* p_parent);

/// tool tip
void ShowAssetToolTip(const AssetMetaData& p_meta, const IAsset* p_asset);

void ShowAssetToolTip(const ContentEntry& p_node);

/// popup
void ShowPopup(const ContentEntry& p_node,
               EditorLayer& p_editor,
               std::filesystem::path* p_rename = nullptr);

}  // namespace cave
