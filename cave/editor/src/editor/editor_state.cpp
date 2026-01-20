#include "editor_state.h"

namespace cave {

void EditorState::OnEnter(const StateRequest& p_args) {
    unused(p_args);
}

void EditorState::Tick(float p_timestep) {
    unused(p_timestep);
}

StateRequest EditorState::PopRequest() {
    return {};
}

}  // namespace cave
