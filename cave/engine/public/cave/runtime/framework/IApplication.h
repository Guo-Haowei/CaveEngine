// =============================================================================
// File: cave/runtime/framework/IApplication.h
// =============================================================================
#pragma once
#include <string_view>

#include "cave/core/Error.h"
#include "cave/core/NonCopyable.h"
#include "cave/rhi/Backend.h"
#include "cave/runtime/framework/AppServices.h"

// clang-format off
namespace cave::render { class Renderer; }
namespace cave::render { class IRenderDevice; }
// clang-format on

namespace cave {

enum class AppStateId : uint8_t;

class AppStateMachine;
class AssetRegistry;
class BootLoadPipeline;
class CommandRegistry;
class Console;
class EventQueue;
class IAssetManager;
class DisplayService;
class ImguiManager;
class InputService;
class IntentDispatcher;
class IUIRuntime;
class IPhysicsManager;
class IScriptService;
class SceneRegistry;
class SceneQueryService;
class SceneScheduler;
class TaskManager;
class ViewManager;

struct AppSpec {
    std::string_view userFolder;
    std::string_view name;
    int width;
    int height;
    rhi::Backend backend;
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

enum class QuitVote : uint8_t {
    Allow,
    Deny,
};

enum class QuitReason : uint8_t {
    WindowClose,
    MenuQuit,
    AltF4,
};

struct QuitContext {
    QuitReason reason;
};

class IApplication : public NonCopyable {
public:
    IApplication(const AppSpec& p_spec)
        : m_spec(p_spec) {
    }

    virtual ~IApplication();

    virtual Result<void> Initialize() = 0;
    virtual void Finalize() = 0;

    virtual QuitVote OnQuitRequested(const QuitContext& p_quit) = 0;

    virtual AppStateId GetStateId() const = 0;
    virtual EventQueue& GetEventQueue() = 0;
    virtual SceneScheduler& GetSceneScheduler() = 0;
    virtual cave::InputService& InputService() = 0;

    // services
    SceneQueryService& SceneQueryService() { return *m_scene_query_service; }

    // @TODO: return reference instead
    AssetRegistry* GetAssetRegistry() { return m_asset_registry; }
    IAssetManager* GetAssetManager() { return m_asset_manager; }
    IUIRuntime* UIService() { return m_ui; }
    SceneRegistry* GetSceneRegistry() { return m_scene_registry; }
    IPhysicsManager* GetPhysicsManager() { return m_physics_manager; }
    IScriptService* ScriptService() { return m_script_service; }
    DisplayService* GetDisplayService() { return m_display_service; }
    render::IRenderDevice* GetRenderDevice() { return m_render_device; }
    ImguiManager* GetImguiManager() { return m_imgui_manager; }
    IntentDispatcher* IntentDispatcher() { return m_intent_dispatcher; }
    TaskManager* GetTaskManager() { return m_task_manager; }
    ViewManager* GetViewManager() { return m_view_manager; }

    CommandRegistry& CommandRegistry() { return *m_cmd_reg; }
    Console& Console() { return *m_console; }

    AppServices& services() { return services_; }

    const AppSpec& GetSpecification() const { return m_spec; }
    rhi::Backend GetBackend() const { return m_spec.backend; }
    bool IsOpenGL() const { return m_spec.backend == rhi::Backend::OpenGL; }

    static void Run(IApplication* p_app);

    // @TODO: get rid of the following
    virtual AppType GetType() const = 0;
    bool IsRuntime() const { return GetType() == AppType::Runtime; }

protected:
    virtual bool MainLoop() = 0;

    AppSpec m_spec;

    cave::SceneQueryService* m_scene_query_service;

    // @TODO: differentiate global and state specific managers
    AssetRegistry* m_asset_registry{};
    IAssetManager* m_asset_manager{};
    SceneRegistry* m_scene_registry{};

    IPhysicsManager* m_physics_manager{};
    IScriptService* m_script_service{};

    DisplayService* m_display_service{};

    render::Renderer* m_renderer{};
    render::IRenderDevice* m_render_device{};

    ImguiManager* m_imgui_manager{};
    cave::IntentDispatcher* m_intent_dispatcher{};
    IUIRuntime* m_ui{};
    TaskManager* m_task_manager{};

    ViewManager* m_view_manager{};

    cave::CommandRegistry* m_cmd_reg{ nullptr };
    cave::Console* m_console{ nullptr };

    AppServices services_;
};

}  // namespace cave
