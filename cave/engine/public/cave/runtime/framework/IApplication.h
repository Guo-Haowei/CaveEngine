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
class EventQueue;

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
        : m_app_spec(spec) {
    }

    virtual ~IApplication() = default;

    virtual Result<void> initialize() = 0;
    virtual void finalize() = 0;

    virtual QuitVote onQuitRequested(const QuitContext& quit) = 0;

    virtual AppStateId stateId() const = 0;

    EngineServices& services() { return m_engine_services; }

    const AppSpec& specification() const { return m_app_spec; }
    rhi::Backend backend() const { return m_app_spec.backend; }

    static void run(IApplication* app);

    // @TODO: clean up
    virtual AppType appType() const = 0;
    bool isRuntime() const { return appType() == AppType::Runtime; }
    virtual EventQueue& eventQueue() = 0;

protected:
    virtual bool mainLoop() = 0;

    AppSpec m_app_spec;

    // @TODO: move the following to services
    // also need subsystems
    EngineServices m_engine_services;
};

}  // namespace cave
