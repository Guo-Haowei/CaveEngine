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
        : spec_(spec) {
    }

    virtual ~IApplication();

    virtual Result<void> Initialize() = 0;
    virtual void Finalize() = 0;

    virtual QuitVote OnQuitRequested(const QuitContext& quit) = 0;

    virtual AppStateId GetStateId() const = 0;
    virtual EventQueue& GetEventQueue() = 0;

    // @TODO: return reference instead
    ImguiManager* GetImguiManager() { return imgui_manager_; }

    CommandRegistry& CommandRegistry() { return *cmd_reg_; }
    Console& Console() { return *console_; }

    EngineServices& services() { return services_; }

    const AppSpec& GetSpecification() const { return spec_; }
    rhi::Backend GetBackend() const { return spec_.backend; }

    static void Run(IApplication* p_app);

    // @TODO: get rid of the following
    virtual AppType GetType() const = 0;
    bool IsRuntime() const { return GetType() == AppType::Runtime; }

protected:
    virtual bool MainLoop() = 0;

    AppSpec spec_;

    // @TODO: move the following to services
    // also need subsystems
    ImguiManager* imgui_manager_{};

    cave::CommandRegistry* cmd_reg_{};
    cave::Console* console_{};

    EngineServices services_;
};

}  // namespace cave
