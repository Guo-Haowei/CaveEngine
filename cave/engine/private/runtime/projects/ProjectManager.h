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

    bool hasProject() const { return project_.is_some(); }

    const ProjectInfo& project() const { return project_.unwrap(); }

    std::string projectRoot() const;

    // @TODO: better snapshot
    TaskSnapshot snapshot() const { return boot_load_pipeline_.rootSnapshot(); }

private:
    VFS& vfs_;
    BootLoadPipeline boot_load_pipeline_;
    render::Renderer& renderer_;

    Option<ProjectInfo> project_;
};

}  // namespace cave
