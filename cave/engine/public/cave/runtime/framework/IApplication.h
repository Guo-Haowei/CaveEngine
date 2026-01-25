// =============================================================================
// File: engine/public/cave/runtime/framework/IApplication.h
// =============================================================================
#pragma once
#include <string_view>

#include "cave/runtime/core/NonCopyable.h"

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
class IGraphicsManager;
class ImguiManager;
class InputSystem;
class IPhysicsManager;
class ISceneManager;
class IScriptManager;
class RenderSystem;
class TaskManager;
class Scene;
class VFS;
class ViewportManager;

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

    virtual ~IApplication() = default;

    virtual Result<void> Initialize() = 0;
    virtual void Finalize() = 0;

    virtual void RequestProject(std::string_view p_path) = 0;

    virtual AppStateId GetStateId() const = 0;
    virtual BootLoadPipeline& GetBootLoadPipeline() = 0;
    virtual VFS& GetVFS() = 0;
    virtual EventQueue& GetEventQueue() = 0;
    virtual GameModeFactory& GetGameModeFactory() = 0;

    AssetRegistry* GetAssetRegistry() { return m_asset_registry; }
    IAssetManager* GetAssetManager() { return m_asset_manager; }
    InputSystem* GetInputSystem() { return m_input_system; }
    ISceneManager* GetSceneManager() { return m_scene_manager; }
    IPhysicsManager* GetPhysicsManager() { return m_physics_manager; }
    IScriptManager* GetScriptManager() { return m_script_manager; }
    IDisplayManager* GetDisplayManager() { return m_display_server; }
    IGraphicsManager* GetGraphicsManager() { return m_graphics_manager; }
    ImguiManager* GetImguiManager() { return m_imgui_manager; }
    RenderSystem* GetRenderSystem() { return m_render_system; }
    TaskManager* GetTaskManager() { return m_task_manager; }
    ViewportManager* GetViewportManager() { return m_viewport_manager; }

    const AppSpec& GetSpecification() const { return m_specification; }

    static void Run(IApplication* p_app);

    // @TODO: get rid of the following
    virtual AppType GetType() const = 0;
    bool IsRuntime() const { return GetType() == AppType::Runtime; }

    virtual bool IsWorld2D() const = 0;

protected:
    virtual bool MainLoop() = 0;

    AppSpec m_specification;

    // @TODO: differentiate global and state specific managers
    AssetRegistry* m_asset_registry{ nullptr };
    IAssetManager* m_asset_manager{ nullptr };
    ISceneManager* m_scene_manager{ nullptr };

    IPhysicsManager* m_physics_manager{ nullptr };
    IScriptManager* m_script_manager{ nullptr };

    IDisplayManager* m_display_server{ nullptr };
    IGraphicsManager* m_graphics_manager{ nullptr };
    RenderSystem* m_render_system{ nullptr };

    ImguiManager* m_imgui_manager{ nullptr };
    InputSystem* m_input_system{ nullptr };
    TaskManager* m_task_manager{ nullptr };

    ViewportManager* m_viewport_manager{ nullptr };
};

}  // namespace cave
