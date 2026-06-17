// =============================================================================
// File: cave/runtime/framework/EngineServices.h
// =============================================================================
#pragma once

// clang-format off
namespace cave::render { class Renderer; }
namespace cave::render { class IRenderDevice; }
// clang-format on

namespace cave {

class AssetRegistry;
class DisplayService;
class IAssetManager;
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

struct EngineServices {
    AssetRegistry* asset_registry_{};
    DisplayService* display_service_{};
    IAssetManager* asset_manager_{};
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

    render::IRenderDevice* render_device_{};
    render::Renderer* renderer_{};

    AssetRegistry& assetRegistry() { return *asset_registry_; }
    DisplayService& displayService() { return *display_service_; }
    IAssetManager& assetManager() { return *asset_manager_; }
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
    render::IRenderDevice& renderDevice() { return *render_device_; };
};

}  // namespace cave
