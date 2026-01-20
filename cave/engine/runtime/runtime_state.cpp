#include "runtime_state.h"

namespace cave {

RuntimeState::RuntimeState(Application& p_app)
    : AppState(p_app) {
}

void RuntimeState:: OnEnter(const StateRequest& p_args) {
    unused(p_args);
}

void RuntimeState::OnExit() {
}

void RuntimeState::Tick(float p_timestep) {
    unused(p_timestep);
}

StateRequest RuntimeState::PopRequest() {
    return {};
}

}  // namespace cave
