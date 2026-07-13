#pragma once
#include "cave/core/containers/StringHash.h"
#include "cave/runtime/assets/AssetHandle.h"

namespace cave {

class DocumentService;
class ThumbnailService;

struct ContentEntry {
    AssetType type;
    AssetHandle handle;

    bool is_dir;
    std::filesystem::path sys_path;
    String virtual_path;
    std::string_view file_name;
    std::string_view extension;

    ContentEntry* parent;
    Vector<Owner<ContentEntry>> children;
};

Owner<ContentEntry> BuildFolderTree(const std::filesystem::path& sys_path,
                                    ContentEntry* parent);

/// tool tip
void ShowAssetToolTip(ThumbnailService& service, const AssetHandle& handle);

void ShowAssetToolTip(ThumbnailService& service, const ContentEntry& node);

/// popup
void ShowPopup(const ContentEntry& node,
               DocumentService& document,
               std::function<void(void)> rename_cb = nullptr);

}  // namespace cave
