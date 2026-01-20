#include "project_browser_state.h"

#include "engine/runtime/application.h"
#include "engine/runtime/imgui_manager.h"

namespace cave {

ProjectBrowserState::ProjectBrowserState(Application& p_app)
    : AppState(p_app) {
}

void ProjectBrowserState::OnEnter(const StateRequest& p_args) {
    unused(p_args);
}

void ProjectBrowserState::OnExit() {
}

void ProjectBrowserState::Tick(float) {
    if (ImguiManager* imgui_manager = m_app.GetImguiManager()) {
        imgui_manager->BeginFrame();

        if (ImGui::Begin("temp")) {
        }
        ImGui::End();

        ImGui::Render();
    }
}

Option<StateRequest> ProjectBrowserState::PopRequest() {
    return None();
}

}  // namespace cave
