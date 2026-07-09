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
class ICanvas;
class IGameInput;
class InputService;
class IntentBus;
class IUIRuntime;
class NativeScriptRegistry;
class ProjectManager;
class SceneRegistry;
class SceneScheduler;
class TaskManager;
class VFS;
class ViewManager;

class GameModuleHandle;

struct RuntimeServices {
    AssetRegistry* asset_registry_{};
    DisplayService* display_service_{};
    IAssetManager* asset_manager_{};
    ICanvas* canvas_{};
    IGameInput* game_input_{};
    InputService* input_service_{};
    IUIRuntime* ui_{};
    NativeScriptRegistry* native_scripts_{};
    ProjectManager* project_manager_{};
    SceneRegistry* scene_registry_{};
    SceneScheduler* scene_scheduler_{};
    TaskManager* task_manager_{};
    ViewManager* view_manager_{};

    AssetRegistry& assetRegistry() { return *asset_registry_; }
    DisplayService& displayService() { return *display_service_; }
    IAssetManager& assetManager() { return *asset_manager_; }
    ICanvas& canvas() { return *canvas_; }
    IGameInput& gameInput() { return *game_input_; }
    InputService& inputService() { return *input_service_; }
    NativeScriptRegistry& nativeScripts() { return *native_scripts_; }
    ProjectManager& projectManager() { return *project_manager_; }
    SceneRegistry& sceneRegistry() { return *scene_registry_; }
    SceneScheduler& sceneScheduler() { return *scene_scheduler_; }
    TaskManager& taskManager() { return *task_manager_; }
    IUIRuntime& ui() { return *ui_; }
    ViewManager& viewManager() { return *view_manager_; }
};

struct EngineServices : public RuntimeServices {
    IntentBus* intent_bus_{};
    VFS* vfs_{};
    GameModuleHandle* game_module_{};

    render::IRenderDevice* render_device_{};
    render::Renderer* renderer_{};

    IntentBus& intentBus() { return *intent_bus_; }
    VFS& vfs() { return *vfs_; }
    GameModuleHandle& gameModule() { return *game_module_; }

    render::Renderer& renderer() { return *renderer_; };
    render::IRenderDevice& renderDevice() { return *render_device_; };
};

}  // namespace cave
