#include "AppState.h"

namespace cave {

void AppStateMachine::initialize(AppStateId p_initial_state) {
    StateRequest req{ p_initial_state };
    state_ = createState(app_, req.next);
    state_->onEnter(req);

    state_id_ = p_initial_state;
}

void AppStateMachine::shutdown() {
    if (DEV_VERIFY(state_)) {
        state_->onExit();
        state_.reset();
    }
}

void AppStateMachine::tick(const FrameTime& p_time) {
    state_->tick(p_time);

    if (auto req = state_->popRequest(); req.is_some()) {
        switchTo(req.unwrap_unchecked());
    }
}

void AppStateMachine::switchTo(const StateRequest& p_request) {
#if USING(DEBUG_BUILD)
    std::string_view old_state = state_->debugId().type;
#endif

    state_->onExit();
    state_ = createState(app_, p_request.next);

    LOG_INFO(LogChannel::App, "State {} -> {}", old_state, state_->debugId().type);

    state_->onEnter(p_request);
    state_id_ = p_request.next;
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
