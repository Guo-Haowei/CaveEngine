#pragma once
#include "engine/core/base/noncopyable.h"
#include "engine/core/os/timer.h"
#include "engine/renderer/graphics_defines.h"
#include "engine/runtime/event_queue.h"
#include "engine/runtime/layer.h"
#include "engine/runtime/module.h"

namespace cave {

class AssetManager;
class AssetRegistry;
class CameraComponent;
class DisplayManager;
class IGraphicsManager;
class ImguiManager;
class InputManager;
class IPhysicsManager;
class ISceneManager;
class IScriptManager;
class RenderSystem;
class Scene;

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
    enum class State : uint8_t {
        EDITING,
        SIM,
        BEGIN_SIM,
        END_SIM,
        COUNT,
    };

    enum class Type : uint32_t {
        RUNTIME,
        EDITOR,
    };

    Application(const ApplicationSpec& p_spec, Type p_type = Type::RUNTIME);
    virtual ~Application() = default;

    auto Initialize(int p_argc, const char** p_argv) -> Result<void>;
    void Finalize();
    static void Run(Application* p_app);

    void AttachLayer(Layer* p_layer);
    void DetachLayer(Layer* p_layer);
    void AttachGameLayer();
    void DetachGameLayer();

    EventQueue& GetEventQueue() { return m_event_queue; }

    AssetRegistry* GetAssetRegistry() { return m_asset_registry; }
    AssetManager* GetAssetManager() { return m_asset_manager; }
    InputManager* GetInputManager() { return m_input_manager; }
    ISceneManager* GetSceneManager() { return m_scene_manager; }
    IPhysicsManager* GetPhysicsManager() { return m_physics_manager; }
    IScriptManager* GetScriptManager() { return m_script_manager; }
    DisplayManager* GetDisplayServer() { return m_display_server; }
    IGraphicsManager* GetGraphicsManager() { return m_graphics_manager; }
    ImguiManager* GetImguiManager() { return m_imgui_manager; }
    RenderSystem* GetRenderSystem() { return m_render_system; }

    const ApplicationSpec& GetSpecification() const { return m_specification; }
    const std::string& GetUserFolder() const { return m_userFolder; }
    const std::string& GetResourceFolder() const { return m_resourceFolder; }

    State GetState() const { return m_state; }
    void SetState(State p_state);

    bool IsRuntime() const { return m_type == Type::RUNTIME; }
    bool IsEditor() const { return m_type == Type::EDITOR; }

    virtual CameraComponent* GetActiveCamera() = 0;

protected:
    [[nodiscard]] auto SetupModules() -> Result<void>;

    bool MainLoop();

    float UpdateTime();

    virtual void RegisterDvars();
    virtual void InitLayers() {}
    // @TODO: add CreateXXXManager for all managers
    virtual Result<ImguiManager*> CreateImguiManager();

    void SaveCommandLine(int p_argc, const char** p_argv);
    void RegisterModule(Module* p_module);

    std::unique_ptr<GameLayer> m_gameLayer;
    std::vector<Layer*> m_layers;

    std::vector<std::string> m_commandLine;
    std::string m_appName;
    std::string m_userFolder;
    std::string m_resourceFolder;
    std::string m_projectFolder;
    ApplicationSpec m_specification;

    EventQueue m_event_queue;

    AssetRegistry* m_asset_registry{ nullptr };
    AssetManager* m_asset_manager{ nullptr };
    ISceneManager* m_scene_manager{ nullptr };
    IPhysicsManager* m_physics_manager{ nullptr };
    DisplayManager* m_display_server{ nullptr };
    IGraphicsManager* m_graphics_manager{ nullptr };
    ImguiManager* m_imgui_manager{ nullptr };
    IScriptManager* m_script_manager{ nullptr };
    RenderSystem* m_render_system{ nullptr };
    InputManager* m_input_manager{ nullptr };

    std::vector<Module*> m_modules;

    const Type m_type;
    State m_state{ State::EDITING };

    Timer m_timer;
};

}  // namespace cave
