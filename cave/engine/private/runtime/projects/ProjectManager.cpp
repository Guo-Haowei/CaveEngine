#include "ProjectManager.h"

#include "cave/core/diagnostics/Log.h"
#include "engine/private/runtime/framework/VFS.h"

namespace cave {

namespace fs = std::filesystem;

ProjectManager::ProjectManager(VFS& vfs,
                               TaskManager& task_manager,
                               IAssetManager& asset_manager,
                               AssetRegistry& asset_registry) noexcept
    : vfs_(vfs)
    , boot_load_pipeline_(task_manager, asset_manager, asset_registry) {
}

void ProjectManager::loadProject(const ProjectInfo& project) {
    DEV_ASSERT(!project.path.empty());
    DEV_ASSERT_MSG(!vfs_.HasMount("@res"), "resource folder already mounted");

    fs::path resource_folder = fs::path(project.path) / "resources";
    vfs_.Mount("@res", resource_folder);

    LOG_INFO(LogChannel::Asset, "+ @{}", resource_folder.string());
    boot_load_pipeline_.RequestProject(resource_folder);
}

}  // namespace cave
