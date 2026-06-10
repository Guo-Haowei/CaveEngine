#include "ProjectManager.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/runtime/framework/IApplication.h"
#include "engine/private/runtime/framework/VFS.h"

namespace cave {

namespace fs = std::filesystem;

ProjectManager::ProjectManager(IApplication& app) noexcept
    : vfs_(app.GetVFS())
    , boot_load_pipeline_(*app.GetTaskManager(),
                          *app.GetAssetManager(),
                          *app.GetAssetRegistry()) {
}

void ProjectManager::loadProject(const ProjectInfo& project) {
    DEV_ASSERT(!project.path.empty());
    DEV_ASSERT_MSG(!vfs_.HasMount("@res"), "resource folder already mounted");

    fs::path resource_folder = fs::path(project.path) / "resources";
    vfs_.Mount("@res", resource_folder);

    LOG_INFO(LogChannel::Asset, "+ @{}", resource_folder.string());
    boot_load_pipeline_.RequestProject(resource_folder);
}

void ProjectManager::unloadProject(const ProjectInfo& project) {
    unused(project);
}

}  // namespace cave
