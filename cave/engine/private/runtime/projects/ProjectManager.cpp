#include "ProjectManager.h"

#include "engine/private/render/renderer/Renderer.h"
#include "engine/private/runtime/framework/VFS.h"

namespace cave {

namespace fs = std::filesystem;

ProjectManager::ProjectManager(VFS& vfs,
                               TaskManager& task_manager,
                               IAssetManager& asset_manager,
                               AssetRegistry& asset_registry,
                               render::Renderer& renderer) noexcept
    : vfs_(vfs)
    , boot_load_pipeline_(task_manager, asset_manager, asset_registry)
    , renderer_(renderer) {
}

void ProjectManager::loadProject(const ProjectInfo& project) {
    DEV_ASSERT(!project.project_root.empty());
    DEV_ASSERT_MSG(!vfs_.HasMount("@res"), "resource folder already mounted");

    fs::path resource_folder = fs::path(project.project_root) / "resources";
    vfs_.Mount("@res", resource_folder);

    boot_load_pipeline_.requestProject(resource_folder);

    project_ = Some(project);
    renderer_.setMode(project.is_2d);
}

std::string ProjectManager::projectRoot() const {
    if (project_.is_none()) {
        return "";
    }
    return project_.unwrap_unchecked().project_root;
}

}  // namespace cave
