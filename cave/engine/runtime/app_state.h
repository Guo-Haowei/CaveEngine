#pragma once

namespace cave {

class Application;

enum class AppStateId : uint8_t {
    ProjectBrowser = 0,
    LoadingScreen,
    EditorMain,
    RuntimeMain,
    Count,
};

struct StateRequest {
    AppStateId next{ AppStateId::ProjectBrowser };
    std::string arg0;  // e.g. project path, error message
};

class AppState {
public:
    AppState(Application& p_app)
        : m_app(p_app) {}

    virtual ~AppState() = default;

    virtual void OnEnter(const StateRequest& p_args) = 0;

    virtual void OnExit() = 0;

    virtual void Tick(float p_timestep) = 0;

    virtual Option<StateRequest> PopRequest() = 0;

#if USING(DEBUG_BUILD)
    virtual const char* GetDebugName() = 0;
#endif

    Application& GetApp() { return m_app; }

protected:
    Application& m_app;
};

class AppStateMachine {
public:
    using CreateFunc = std::unique_ptr<AppState> (*)(Application&);

    AppStateMachine(Application& p_app)
        : m_app(p_app) {}

    void Init(AppStateId p_initial_state);

    void Shutdown();

    void Tick(float p_timestep);

    AppStateId GetStateId() const { return m_state_id; }

    static void RegisterCreateFunc(AppStateId p_state_id, CreateFunc p_func);

    static std::unique_ptr<AppState> CreateState(Application& p_app, AppStateId p_state_id);

private:
    void SwitchTo(const StateRequest& p_request);

    inline static CreateFunc s_create_funcs[std::to_underlying(AppStateId::Count)];

    Application& m_app;
    std::unique_ptr<AppState> m_state;
    AppStateId m_state_id;
};

}  // namespace cave
