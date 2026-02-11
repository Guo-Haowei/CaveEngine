#pragma once
#include "engine/private/runtime/assets/AssetHandle.h"

namespace cave {

class EditorState;
class ThumbnailService;

struct ContentEntry {
    AssetType type;
    AssetHandle handle;

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
void ShowAssetToolTip(ThumbnailService& p_service, const AssetHandle& p_handle);

void ShowAssetToolTip(ThumbnailService& p_service, const ContentEntry& p_node);

/// popup
void ShowPopup(const ContentEntry& p_node,
               EditorState& p_editor,
               std::function<void(void)> p_rename_cb = nullptr);

}  // namespace cave
