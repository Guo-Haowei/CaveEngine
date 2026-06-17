// =============================================================================
// File: cave/runtime/framework/IApplication.h
// =============================================================================
#pragma once
#include <string_view>

#include "cave/core/error/Result.h"
#include "cave/core/base/NonCopyable.h"
#include "cave/rhi/Backend.h"
#include "cave/runtime/framework/EngineServices.h"

namespace cave {

enum class AppStateId : uint8_t;

class AppStateMachine;
class CommandRegistry;
class Console;
class EventQueue;
class ImguiManager;
class IPhysicsManager;

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
    IPhysicsManager* GetPhysicsManager() { return m_physics_manager; }
    ImguiManager* GetImguiManager() { return m_imgui_manager; }

    CommandRegistry& CommandRegistry() { return *m_cmd_reg; }
    Console& Console() { return *m_console; }

    EngineServices& services() { return services_; }

    const AppSpec& GetSpecification() const { return m_spec; }
    rhi::Backend GetBackend() const { return m_spec.backend; }

    static void Run(IApplication* p_app);

    // @TODO: get rid of the following
    virtual AppType GetType() const = 0;
    bool IsRuntime() const { return GetType() == AppType::Runtime; }

protected:
    virtual bool MainLoop() = 0;

    AppSpec m_spec;

    // @TODO: move the following to services
    // also need subsystems
    IPhysicsManager* m_physics_manager{};
    ImguiManager* m_imgui_manager{};

    cave::CommandRegistry* m_cmd_reg{ nullptr };
    cave::Console* m_console{ nullptr };

    EngineServices services_;
};

}  // namespace cave
