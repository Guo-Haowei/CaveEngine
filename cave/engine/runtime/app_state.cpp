#include "app_state.h"

namespace cave {

void AppStateMachine::Init(AppStateId p_initial_state) {
    StateRequest req{ p_initial_state };
    m_state = CreateState(m_app, req.next);
    m_state->OnEnter(req);

    m_state_id = p_initial_state;
}

void AppStateMachine::Shutdown() {
    if (DEV_VERIFY(m_state)) {
        m_state->OnExit();
    }
}

void AppStateMachine::Tick(float p_timestep) {
    m_state->Tick(p_timestep);

    if (auto req = m_state->PopRequest(); req.is_some()) {
        SwitchTo(req.unwrap_unchecked());
    }
}

void AppStateMachine::SwitchTo(const StateRequest& p_request) {
#if USING(DEBUG_BUILD)
    const char* old_state = m_state->GetDebugName();
#endif

    m_state->OnExit();
    m_state = CreateState(m_app, p_request.next);
    m_state->OnEnter(p_request);

    m_state_id = p_request.next;

#if USING(DEBUG_BUILD)
    const char* new_state = m_state->GetDebugName();
    LOG("AppStateMachine::SwitchTo: {} -> {}", old_state, new_state);
#endif
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
