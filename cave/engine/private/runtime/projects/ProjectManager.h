#pragma once
#include "ProjectInfo.h"

#include "engine/private/runtime/framework/BootLoadPipeline.h"

namespace cave {

class IApplication;
class VFS;

class ProjectManager {
public:
    ProjectManager(VFS& vfs,
                   TaskManager& task_manager,
                   IAssetManager& asset_manager,
                   AssetRegistry& asset_registry) noexcept;

    void loadProject(const ProjectInfo& project);

    bool hasProject() const { return project_.is_some(); }

    const ProjectInfo& project() const { return project_.unwrap(); }

    // @TODO: better snapshot
    TaskSnapshot snapshot() const { return boot_load_pipeline_.RootSnapshot(); }

private:
    VFS& vfs_;
    BootLoadPipeline boot_load_pipeline_;

    Option<ProjectInfo> project_;
};

}  // namespace cave
