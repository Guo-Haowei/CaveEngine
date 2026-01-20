#include "app_state.h"

namespace cave {

void AppStateMachine::Init(Application& p_app, AppStateId p_initial_state) {
    StateRequest req{ true, p_initial_state };
    m_state = CreateState(req.next);
    m_state->OnEnter(p_app, req);
}

void AppStateMachine::Tick(Application& p_app, float p_timestep) {
    m_state->Tick(p_app, p_timestep);

    // Apply transition once per frame, at a controlled point
    if (StateRequest req = m_state->PopRequest(); req.requested) {
        SwitchTo(p_app, req);
    }
}

void AppStateMachine::SwitchTo(Application& p_app, const StateRequest& p_request) {
    m_state->OnExit(p_app);
    m_state = CreateState(p_request.next);
    m_state->OnEnter(p_app, p_request);
}

// static std::unique_ptr<IAppState> AppStateMachine::CreateState(AppStateId p_state_id);

}  // namespace cave
