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
class IPhysicsManager;
class IScriptService;
class SceneRegistry;

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
    IApplication(const AppSpec& spec)
        : m_spec(spec) {
    }

    virtual ~IApplication();

    virtual Result<void> Initialize() = 0;
    virtual void Finalize() = 0;

    virtual QuitVote OnQuitRequested(const QuitContext& quit) = 0;

    virtual AppStateId GetStateId() const = 0;
    virtual EventQueue& GetEventQueue() = 0;

    // @TODO: return reference instead
    AssetRegistry* GetAssetRegistry() { return m_asset_registry; }
    IAssetManager* GetAssetManager() { return m_asset_manager; }
    IPhysicsManager* GetPhysicsManager() { return m_physics_manager; }
    IScriptService* ScriptService() { return m_script_service; }
    DisplayService* GetDisplayService() { return m_display_service; }
    render::IRenderDevice* GetRenderDevice() { return m_render_device; }
    ImguiManager* GetImguiManager() { return m_imgui_manager; }

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

    // @TODO: differentiate global and state specific managers
    AssetRegistry* m_asset_registry{};
    IAssetManager* m_asset_manager{};

    IPhysicsManager* m_physics_manager{};
    IScriptService* m_script_service{};

    DisplayService* m_display_service{};

    render::IRenderDevice* m_render_device{};

    ImguiManager* m_imgui_manager{};

    cave::CommandRegistry* m_cmd_reg{ nullptr };
    cave::Console* m_console{ nullptr };

    AppServices services_;
};

}  // namespace cave
