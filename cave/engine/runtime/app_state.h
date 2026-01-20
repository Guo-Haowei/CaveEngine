#pragma once

namespace cave {

class Application;

enum class AppStateId {
    ProjectBrowser = 0,
    LoadingScreen,
    EditorMain,
    RuntimeMain,
};

struct StateRequest {
    bool requested{ false };
    AppStateId next{ AppStateId::ProjectBrowser };
    std::string arg0;  // e.g. project path, error message
};

struct IAppState {
    virtual ~IAppState() = default;

    virtual void OnEnter(Application& p_app, const StateRequest& p_args) {
        unused(p_app);
        unused(p_args);
    }

    virtual void OnExit(Application& p_app) {
        unused(p_app);
    }

    virtual void Tick(Application& p_app, float p_timestep) = 0;

    virtual StateRequest PopRequest() {
        return {};
    }
};

class AppStateMachine {
public:
    void Init(Application& p_app, AppStateId p_initial_state);

    void Tick(Application& p_app, float p_timestep);

private:
    void SwitchTo(Application& p_app, const StateRequest& p_request);

    static std::unique_ptr<IAppState> CreateState(AppStateId p_state_id);

    std::unique_ptr<IAppState> m_state;
};

}  // namespace cave
