#include "runtime_state.h"

#include <imgui/imgui.h>

#include "engine/runtime/application.h"
#include "engine/runtime/imgui_manager.h"
#include "engine/runtime/script_manager.h"
#include "engine/scene/scene.h"
#include "engine/scene/scene_manager.h"

//
#include "engine/runtime/graphics_manager_interface.h"

namespace cave {

RuntimeState::RuntimeState(Application& p_app)
    : AppState(p_app) {
}

void RuntimeState::OnEnter(const StateRequest& p_args) {
    unused(p_args);

    std::shared_ptr<Scene> current_scene = m_app.GetSceneManager()->GetActiveScene();
    std::shared_ptr<Scene> sim_scene = std::make_shared<Scene>();
    sim_scene->Copy(*current_scene);
    sim_scene->Update(0.0f);

    m_app.GetSceneManager()->OpenSimScene(sim_scene);

    // set active scene?
    // game_layer->SetActiveScene(std::move(sim_scene));
    m_app.GetScriptManager()->OnSimBegin(*sim_scene);
}

void RuntimeState::OnExit() {
    // scene_manager.CloseSimScene();
    m_app.GetScriptManager()->OnSimEnd();
}

void RuntimeState::Tick(float p_timestep) {
    if (ImguiManager* imgui_manager = m_app.GetImguiManager()) {
        imgui_manager->BeginFrame();

        if (ImGui::Begin("temp window for display")) {
            const IGraphicsManager* gm = GetApp().GetGraphicsManager();
            uint64_t handle = gm->GetFinalImage();

            ImGui::Image((ImTextureID)handle, ImVec2{800, 600});
        }
        ImGui::End();

        ImGui::Render();
    }

    if (std::shared_ptr<Scene> scene = m_app.GetSceneManager()->GetActiveScene()) {
        m_app.GetScriptManager()->Update(*scene, p_timestep);
    }
}

Option<StateRequest> RuntimeState::PopRequest() {
    return None();
}

}  // namespace cave
