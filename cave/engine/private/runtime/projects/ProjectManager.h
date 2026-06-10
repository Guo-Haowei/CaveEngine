#pragma once
#include "ProjectInfo.h"

#include "engine/private/runtime/framework/BootLoadPipeline.h"

namespace cave {

class IApplication;
class VFS;

class ProjectManager {
public:
    ProjectManager(IApplication& app) noexcept;

    void loadProject(const ProjectInfo& project);

    void unloadProject(const ProjectInfo& project);

    // @TODO: better snapshot
    TaskSnapshot snapshot() const { return boot_load_pipeline_.RootSnapshot(); }

private:
    VFS& vfs_;
    BootLoadPipeline boot_load_pipeline_;
};

}  // namespace cave
