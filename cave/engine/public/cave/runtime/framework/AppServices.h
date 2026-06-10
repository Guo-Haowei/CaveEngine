#pragma once
// =============================================================================
// File: cave/runtime/framework/AppServices.h
// =============================================================================
#pragma once

namespace cave {

class InputService;
class IUIRuntime;
class ProjectManager;
class SceneRegistry;
class SceneScheduler;
class SceneQueryService;
class TaskManager;
class VFS;
class ViewManager;

struct AppServices {
    InputService* input_service_{};
    IUIRuntime* ui_{};
    ProjectManager* project_manager_{};
    SceneQueryService* scene_query_{};
    SceneRegistry* scene_registry_{};
    SceneScheduler* scene_scheduler_{};
    TaskManager* task_manager_{};
    ViewManager* view_manager_{};
    VFS* vfs_{};

    InputService& inputService() { return *input_service_; }
    IUIRuntime& ui() { return *ui_; }
    ProjectManager& projectManager() { return *project_manager_; }
    SceneQueryService& sceneQuery() { return *scene_query_; }
    SceneRegistry& sceneRegistry() { return *scene_registry_; }
    SceneScheduler& sceneScheduler() { return *scene_scheduler_; }
    TaskManager& taskManager() { return *task_manager_; }
    ViewManager& viewManager() { return *view_manager_; }
    VFS& vfs() { return *vfs_; }
};

}  // namespace cave
