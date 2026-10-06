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
    : m_vfs(vfs)
    , m_boot_load_pipeline(task_manager, asset_manager, asset_registry)
    , m_renderer(renderer) {
}

void ProjectManager::loadProject(const ProjectInfo& project) {
    DEV_ASSERT(!project.project_root.empty());
    DEV_ASSERT_MSG(!m_vfs.HasMount("@res"), "resource folder already mounted");

    fs::path resource_folder = fs::path(project.project_root) / "resources";
    m_vfs.Mount("@res", resource_folder);

    auto res = m_boot_load_pipeline.requestProject(resource_folder);
    DEV_ASSERT(res);

    m_project = Some(project);
    m_renderer.setMode(project.is_2d);
}

std::string ProjectManager::projectRoot() const {
    if (m_project.is_none()) {
        return "";
    }
    return m_project.unwrap_unchecked().project_root;
}

}  // namespace cave
