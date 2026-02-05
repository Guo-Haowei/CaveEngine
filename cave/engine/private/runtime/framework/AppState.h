#pragma once
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
    std::string arg0;  // e.g. project path, error message
};

class AppState {
public:
    AppState(IApplication& p_app)
        : m_app(p_app) {}

    virtual ~AppState() = default;

    virtual void OnEnter(const StateRequest& p_args) = 0;

    virtual void OnExit() = 0;

    virtual void Tick(const FrameTime& p_time) = 0;

    virtual Option<StateRequest> PopRequest() = 0;

#if USING(DEBUG_BUILD)
    virtual const char* GetDebugName() = 0;
#endif

    IApplication& GetApp() { return m_app; }

protected:
    IApplication& m_app;
};

class AppStateMachine {
public:
    using CreateFunc = std::unique_ptr<AppState> (*)(IApplication&);

    AppStateMachine(IApplication& p_app)
        : m_app(p_app) {}

    void Init(AppStateId p_initial_state);

    void Shutdown();

    void Tick(const FrameTime& p_time);

    AppStateId GetStateId() const { return m_state_id; }

    AppState* GetAppState() const { return m_state.get(); }

    static void RegisterCreateFunc(AppStateId p_state_id, CreateFunc p_func);

    static std::unique_ptr<AppState> CreateState(IApplication& p_app, AppStateId p_state_id);

private:
    void SwitchTo(const StateRequest& p_request);

    inline static CreateFunc s_create_funcs[std::to_underlying(AppStateId::Count)];

    IApplication& m_app;
    std::unique_ptr<AppState> m_state;
    AppStateId m_state_id;
};

}  // namespace cave
