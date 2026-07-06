#include "AppState.h"

namespace cave {

void AppStateMachine::initialize(AppStateId p_initial_state) {
    StateRequest req{ p_initial_state };
    m_app_state = createState(m_app, req.next);
    m_app_state->onEnter(req);

    m_state_id = p_initial_state;
}

void AppStateMachine::shutdown() {
    if (DEV_VERIFY(m_app_state)) {
        m_app_state->onExit();
        m_app_state.reset();
    }
}

void AppStateMachine::tick(const FrameTime& p_time) {
    m_app_state->tick(p_time);

    if (auto req = m_app_state->popRequest()) {
        switchTo(req.unwrap_unchecked());
    }
}

void AppStateMachine::switchTo(const StateRequest& p_request) {
#if USING(DEBUG_BUILD)
    std::string_view old_state = m_app_state->debugId().type;
#endif

    m_app_state->onExit();
    m_app_state = createState(m_app, p_request.next);

    LOG_INFO(LogChannel::App, "State {} -> {}", old_state, m_app_state->debugId().type);

    m_app_state->onEnter(p_request);
    m_state_id = p_request.next;
}

void AppStateMachine::registerCreateFunc(AppStateId p_state_id, CreateFunc p_func) {
    const uint8_t index = std::to_underlying(p_state_id);
    DEV_ASSERT(s_create_funcs[index] == nullptr);

    s_create_funcs[index] = p_func;
}

std::unique_ptr<AppState> AppStateMachine::createState(IApplication& p_app, AppStateId p_state_id) {
    const uint8_t index = std::to_underlying(p_state_id);
    if (index >= std::to_underlying(AppStateId::Count) || s_create_funcs[index] == nullptr) {
        return nullptr;
    }

    return s_create_funcs[index](p_app);
}

}  // namespace cave
