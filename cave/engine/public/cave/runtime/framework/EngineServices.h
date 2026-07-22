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
class CommandRegistry;
class Console;
class DisplayService;
class IAssetManager;
class ICanvas;
class IGameInput;
class ImGuiService;
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
    AssetRegistry* asset_registry{};
    Console* console_{};
    CommandRegistry* command_registry{};
    DisplayService* display_service{};
    ICanvas* canvas_{};
    IGameInput* game_input{};
    InputService* input_service{};
    IUIRuntime* ui{};
    NativeScriptRegistry* native_scripts{};
    ProjectManager* project_manager{};
    SceneRegistry* scene_registry{};
    SceneScheduler* scene_scheduler{};
    TaskManager* task_manager{};
    ViewManager* view_manager{};

    // optional
    ImGuiService* imgui{};

    AssetRegistry& assetRegistry() { return *asset_registry; }
    Console& console() { return *console_; }
    CommandRegistry& commandRegistry() { return *command_registry; }
    DisplayService& displayService() { return *display_service; }
    ICanvas& canvas() { return *canvas_; }
    IGameInput& gameInput() { return *game_input; }
    InputService& inputService() { return *input_service; }
    NativeScriptRegistry& nativeScripts() { return *native_scripts; }
    ProjectManager& projectManager() { return *project_manager; }
    SceneRegistry& sceneRegistry() { return *scene_registry; }
    SceneScheduler& sceneScheduler() { return *scene_scheduler; }
    TaskManager& taskManager() { return *task_manager; }
    IUIRuntime& UI() { return *ui; }
    ViewManager& viewManager() { return *view_manager; }
};

struct EngineServices : public RuntimeServices {
    IAssetManager* asset_manager{};
    IntentBus* intent_bus{};
    VFS* vfs{};
    GameModuleHandle* game_module{};
    ICanvas* ui_canvas{};

    render::IRenderDevice* render_device{};
    render::Renderer* renderer_{};

    IAssetManager& assetManager() { return *asset_manager; }
    IntentBus& intentBus() { return *intent_bus; }
    VFS& VFS() { return *vfs; }
    GameModuleHandle& gameModule() { return *game_module; }
    ICanvas& UICanvas() { return *ui_canvas; }

    render::Renderer& renderer() { return *renderer_; };
    render::IRenderDevice& renderDevice() { return *render_device; };
};

}  // namespace cave
