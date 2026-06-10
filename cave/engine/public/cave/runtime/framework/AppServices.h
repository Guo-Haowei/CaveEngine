#pragma once
// =============================================================================
// File: cave/runtime/framework/AppServices.h
// =============================================================================
#pragma once

// clang-format off
namespace cave::render { class Renderer; }
// clang-format on

namespace cave {

class InputService;
class IntentDispatcher;
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
    IntentDispatcher* intent_dispatcher_{};
    IUIRuntime* ui_{};
    ProjectManager* project_manager_{};
    SceneQueryService* scene_query_{};
    SceneRegistry* scene_registry_{};
    SceneScheduler* scene_scheduler_{};
    TaskManager* task_manager_{};
    ViewManager* view_manager_{};
    VFS* vfs_{};
    render::Renderer* renderer_{};

    InputService& inputService() { return *input_service_; }
    IntentDispatcher& intentDispatcher() { return *intent_dispatcher_; }
    IUIRuntime& ui() { return *ui_; }
    ProjectManager& projectManager() { return *project_manager_; }
    SceneQueryService& sceneQuery() { return *scene_query_; }
    SceneRegistry& sceneRegistry() { return *scene_registry_; }
    SceneScheduler& sceneScheduler() { return *scene_scheduler_; }
    TaskManager& taskManager() { return *task_manager_; }
    ViewManager& viewManager() { return *view_manager_; }
    VFS& vfs() { return *vfs_; }
    render::Renderer& renderer() { return *renderer_; };
};

}  // namespace cave
