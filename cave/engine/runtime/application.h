#pragma once
#include "engine/core/base/noncopyable.h"
#include "engine/core/os/timer.h"
#include "engine/renderer/graphics_defines.h"
#include "engine/runtime/event_queue.h"
#include "engine/runtime/module.h"

namespace cave {

enum class AppStateId : uint8_t;

class AppStateMachine;
class IAssetManager;
class AssetRegistry;
class CameraComponent;
class IDisplayManager;
class IGraphicsManager;
class ImguiManager;
class InputManager;
class IPhysicsManager;
class ISceneManager;
class IScriptManager;
class RenderSystem;
class TaskManager;
class Scene;
class VFS;
class ViewportManager;

struct ApplicationSpec {
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

class Application : public NonCopyable {
public:
    enum class Type : uint32_t {
        Runtime,
        Editor,
        Tool,
    };

    Application(const ApplicationSpec& p_spec, Type p_type = Type::Runtime);
    virtual ~Application();

    virtual Result<void> Initialize();
    virtual void Finalize();
    static void Run(Application* p_app);

    AppStateId GetStateId() const;

    EventQueue& GetEventQueue() { return m_event_queue; }

    AssetRegistry* GetAssetRegistry() { return m_asset_registry; }
    IAssetManager* GetAssetManager() { return m_asset_manager; }
    InputManager* GetInputManager() { return m_input_manager; }
    ISceneManager* GetSceneManager() { return m_scene_manager; }
    IPhysicsManager* GetPhysicsManager() { return m_physics_manager; }
    IScriptManager* GetScriptManager() { return m_script_manager; }
    IDisplayManager* GetDisplayServer() { return m_display_server; }
    IGraphicsManager* GetGraphicsManager() { return m_graphics_manager; }
    ImguiManager* GetImguiManager() { return m_imgui_manager; }
    RenderSystem* GetRenderSystem() { return m_render_system; }
    ViewportManager* GetViewportManager() { return m_viewport_manager; }
    VFS& GetVFS() { return *m_vfs; }

    const ApplicationSpec& GetSpecification() const { return m_specification; }

    void LoadProjectAsync(std::string_view p_path);

    // @TODO: get rid of the following
    bool IsRuntime() const { return m_type == Type::Runtime; }
    bool IsEditor() const { return m_type == Type::Editor; }
    virtual bool IsWorld2D() const = 0;

protected:
    [[nodiscard]] auto SetupModules() -> Result<void>;

    virtual bool MainLoop();

    float UpdateTime();

    // @TODO: add CreateXXXManager for all managers
    virtual Result<ImguiManager*> CreateImguiManager();

    void RegisterModule(Module* p_module);

    const Type m_type;

    ApplicationSpec m_specification;

    EventQueue m_event_queue;

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
    InputManager* m_input_manager{ nullptr };
    TaskManager* m_task_manager{ nullptr };

    ViewportManager* m_viewport_manager{ nullptr };

    std::unique_ptr<VFS> m_vfs;

    std::vector<Module*> m_modules;

    Timer m_timer;

    std::unique_ptr<AppStateMachine> m_state_machine;
};

}  // namespace cave
