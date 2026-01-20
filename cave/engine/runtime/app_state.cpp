#include "app_state.h"

namespace cave {

void AppStateMachine::Init(AppStateId p_initial_state) {
    StateRequest req{ true, p_initial_state };
    m_state = CreateState(m_app, req.next);
    m_state->OnEnter(req);
}

void AppStateMachine::Tick(float p_timestep) {
    m_state->Tick(p_timestep);

    if (StateRequest req = m_state->PopRequest(); req.requested) {
        SwitchTo(req);
    }
}

void AppStateMachine::SwitchTo(const StateRequest& p_request) {
    m_state->OnExit();
    m_state = CreateState(m_app, p_request.next);
    m_state->OnEnter(p_request);
}

void AppStateMachine::RegisterCreateFunc(AppStateId p_state_id, CreateFunc p_func) {
    const uint8_t index = std::to_underlying(p_state_id);
    DEV_ASSERT(s_create_funcs[index] == nullptr);

    s_create_funcs[index] = p_func;
}

std::unique_ptr<AppState> AppStateMachine::CreateState(Application& p_app, AppStateId p_state_id) {
    const uint8_t index = std::to_underlying(p_state_id);
    if (index >= std::to_underlying(AppStateId::Count) || s_create_funcs[index] == nullptr) {
        return nullptr;
    }

    return s_create_funcs[index](p_app);
}

}  // namespace cave
