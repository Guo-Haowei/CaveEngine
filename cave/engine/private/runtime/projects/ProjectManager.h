#pragma once
#include "ProjectInfo.h"

#include "engine/private/runtime/framework/BootLoadPipeline.h"

// clang-format off
namespace cave::render { class Renderer; }
// clang-format on

namespace cave {

class IApplication;
class VFS;

class ProjectManager {
public:
    ProjectManager(VFS& vfs,
                   TaskManager& task_manager,
                   IAssetManager& asset_manager,
                   AssetRegistry& asset_registry,
                   render::Renderer& renderer) noexcept;

    void loadProject(const ProjectInfo& project);

    bool hasProject() const { return m_project.is_some(); }

    const ProjectInfo& project() const { return m_project.unwrap(); }

    std::string projectRoot() const;

    // @TODO: better snapshot
    TaskSnapshot snapshot() const { return m_boot_load_pipeline.rootSnapshot(); }

private:
    VFS& m_vfs;
    BootLoadPipeline m_boot_load_pipeline;
    render::Renderer& m_renderer;

    Option<ProjectInfo> m_project;
};

}  // namespace cave
