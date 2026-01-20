#pragma once

namespace cave {

class Application;

enum class AppStateId {
    ProjectBrowser = 0,
    LoadingScreen,
    EditorMain,
    RuntimeMain,
    Count,
};

struct StateRequest {
    bool requested{ false };
    AppStateId next{ AppStateId::ProjectBrowser };
    std::string arg0;  // e.g. project path, error message
};

class AppState {
public:
    AppState(Application& p_app)
        : m_app(p_app) {}

    virtual ~AppState() = default;

    virtual void OnEnter(const StateRequest& p_args) { unused(p_args); }

    virtual void OnExit() {}

    virtual void Tick(float p_timestep) = 0;

    virtual StateRequest PopRequest() { return {}; }

protected:
    Application& m_app;
};

class AppStateMachine {
public:
    using CreateFunc = std::unique_ptr<AppState> (*)(Application&);

    AppStateMachine(Application& p_app)
        : m_app(p_app) {}

    void Init(AppStateId p_initial_state);

    void Tick(float p_timestep);

    static void RegisterCreateFunc(AppStateId p_state_id, CreateFunc p_func);

    static std::unique_ptr<AppState> CreateState(Application& p_app, AppStateId p_state_id);

private:
    void SwitchTo(const StateRequest& p_request);

    static CreateFunc s_create_funcs[std::to_underlying(AppStateId::Count)];

    Application& m_app;
    std::unique_ptr<AppState> m_state;
};

}  // namespace cave
