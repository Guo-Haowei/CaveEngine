// =============================================================================
// File: engine/public/cave/runtime/framework/IApplication.h
// =============================================================================
#pragma once
#include <string_view>

#include "cave/core/Error.h"
#include "cave/core/NonCopyable.h"

// clang-format off
namespace cave::render { class Renderer; }
namespace cave::render { class IRenderDevice; }
// clang-format on

namespace cave {

enum class AppStateId : uint8_t;
enum class Backend : uint8_t;

class AppStateMachine;
class AssetRegistry;
class BootLoadPipeline;
class EventQueue;
class GameModeFactory;
class IAssetManager;
class IDisplayManager;
class ImguiManager;
class InputSystem;
class IPhysicsManager;
class IScriptManager;
class ISceneRegistry;
class SceneQueryService;
class SceneScheduler;
class TaskManager;
class VFS;
class ViewManager;

struct AppSpec {
    std::string_view userFolder;
    std::string_view name;
    int width;
    int height;
    Backend backend;
    bool decorated;
    bool fullscreen;
    bool vsync;
    bool enableImgui;
};

enum class AppType : uint8_t {
    Runtime,
    Editor,
    Tool,
};

class IApplication : public NonCopyable {
public:
    IApplication(const AppSpec& p_spec)
        : m_specification(p_spec) {
    }

    virtual ~IApplication();

    virtual Result<void> Initialize() = 0;
    virtual void Finalize() = 0;

    virtual void RequestProject(std::string_view p_path) = 0;

    virtual AppStateId GetStateId() const = 0;
    virtual BootLoadPipeline& GetBootLoadPipeline() = 0;
    virtual VFS& GetVFS() = 0;
    virtual EventQueue& GetEventQueue() = 0;
    virtual GameModeFactory& GetGameModeFactory() = 0;
    virtual SceneScheduler& GetSceneScheduler() = 0;

    // services
    SceneQueryService& SceneQueryService() { return *m_scene_query_service; }

    // @TODO: return reference instead
    AssetRegistry* GetAssetRegistry() { return m_asset_registry; }
    IAssetManager* GetAssetManager() { return m_asset_manager; }
    InputSystem* GetInputSystem() { return m_input_system; }
    ISceneRegistry* GetSceneRegistry() { return m_scene_registry; }
    IPhysicsManager* GetPhysicsManager() { return m_physics_manager; }
    IScriptManager* GetScriptManager() { return m_script_manager; }
    IDisplayManager* GetDisplayManager() { return m_display_server; }
    render::IRenderDevice* GetRenderDevice() { return m_render_device; }
    ImguiManager* GetImguiManager() { return m_imgui_manager; }
    TaskManager* GetTaskManager() { return m_task_manager; }
    ViewManager* GetViewManager() { return m_view_manager; }

    const AppSpec& GetSpecification() const { return m_specification; }

    static void Run(IApplication* p_app);

    // @TODO: get rid of the following
    virtual AppType GetType() const = 0;
    bool IsRuntime() const { return GetType() == AppType::Runtime; }

    virtual bool IsWorld2D() const = 0;

protected:
    virtual bool MainLoop() = 0;

    AppSpec m_specification;

    cave::SceneQueryService* m_scene_query_service;

    // @TODO: differentiate global and state specific managers
    AssetRegistry* m_asset_registry{ nullptr };
    IAssetManager* m_asset_manager{ nullptr };
    ISceneRegistry* m_scene_registry{ nullptr };

    IPhysicsManager* m_physics_manager{ nullptr };
    IScriptManager* m_script_manager{ nullptr };

    IDisplayManager* m_display_server{ nullptr };

    render::Renderer* m_renderer{ nullptr };
    render::IRenderDevice* m_render_device{ nullptr };

    ImguiManager* m_imgui_manager{ nullptr };
    InputSystem* m_input_system{ nullptr };
    TaskManager* m_task_manager{ nullptr };

    ViewManager* m_view_manager{ nullptr };
};

}  // namespace cave
