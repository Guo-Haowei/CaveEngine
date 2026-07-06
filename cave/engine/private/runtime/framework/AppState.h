#pragma once
#include "cave/core/ids/DebugId.h"
#include "cave/core/time/FrameTime.h"

namespace cave {

class IApplication;

enum class AppStateId : uint8_t {
    ProjectBrowser = 0,
    LoadingScreen,
    Editor,
    GameRuntime,
    Count,
};

struct StateRequest {
    AppStateId next{ AppStateId::ProjectBrowser };
    std::string arg0;
    std::string arg1;
};

class AppState {
public:
    AppState(IApplication& app)
        : m_app(app) {}

    virtual ~AppState() = default;

    virtual void onEnter(const StateRequest& args) = 0;

    virtual void onExit() = 0;

    virtual void tick(const FrameTime& time) = 0;

    virtual Option<StateRequest> popRequest() = 0;

#if USING(DEBUG_BUILD)
    virtual DebugId debugId() const = 0;
#endif

    IApplication& app() { return m_app; }

protected:
    IApplication& m_app;
};

class AppStateMachine {
public:
    using CreateFunc = std::unique_ptr<AppState> (*)(IApplication&);

    AppStateMachine(IApplication& app)
        : m_app(app) {}

    void initialize(AppStateId initial_state);

    void shutdown();

    void tick(const FrameTime& time);

    AppStateId stateId() const { return m_state_id; }

    AppState* appState() const { return m_app_state.get(); }

    static void registerCreateFunc(AppStateId state_id, CreateFunc func);

    static auto createState(IApplication& app, AppStateId state_id)
        -> std::unique_ptr<AppState>;

private:
    void switchTo(const StateRequest& request);

    inline static CreateFunc s_create_funcs[std::to_underlying(AppStateId::Count)];

    IApplication& m_app;
    std::unique_ptr<AppState> m_app_state;
    AppStateId m_state_id;
};

}  // namespace cave
