#pragma once
// =============================================================================
// File: cave/runtime/framework/AppServices.h
// =============================================================================
#pragma once

namespace cave {

class ProjectManager;
class VFS;

struct AppServices {
    VFS* vfs_ = nullptr;
    ProjectManager* project_manager_ = nullptr;

    ProjectManager& projectManager() { return *project_manager_; }
    VFS& vfs() { return *vfs_; }
};

}  // namespace cave
