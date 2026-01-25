// =============================================================================
// File: engine/private/runtime/gameplay/GameRuntimeState.cpp
// =============================================================================
#include "GameRuntimeState.h"

#include <imgui/imgui.h>

#include "engine/private/runtime/framework/Application.h"
#include "engine/private/runtime/framework/IGraphicsManager.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/framework/InputSystem.h"
#include "engine/private/runtime/framework/ISceneManager.h"
#include "engine/private/runtime/framework/ScriptManager.h"
#include "engine/private/runtime/framework/ViewportManager.h"

// @TODO: refactor
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/scene/scene.h"
#include "engine/private/scene/scene_manager.h"

namespace cave {

static bool BeginFullscreenWindow(const char* p_name) {
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoDocking;

    return ImGui::Begin(p_name, nullptr, flags);
}

static void EndFullscreenWindow() {
    ImGui::End();
    ImGui::PopStyleVar(3);
}

class RuntimeSceneViewProvider : public ISceneViewProvider {
public:
    RuntimeSceneViewProvider(Application& p_app)
        : m_app(p_app) {}

    void BuildViews(std::vector<SceneView>& p_out_views, bool p_is_opengl) final {
        std::shared_ptr<Scene> scene = m_app.GetSceneManager()->GetActiveScene();

        // @HACK: find the first non-editor camera
        for (auto [id, camera] : scene->View<CameraComponent>()) {
            if (scene->Contains<NoSaveTag>(id)) {
                continue;
            }

            SceneView scene_view;
            scene_view.scene = scene.get();

            ViewInfo::FromCamera(camera,
                                 scene_view.view_info,
                                 p_is_opengl);

            p_out_views.push_back(scene_view);
            break;
        }
    }

private:
    Application& m_app;
};

GameRuntimeState::GameRuntimeState(Application& p_app)
    : AppState(p_app) {
}

void GameRuntimeState::OnEnter(const StateRequest& p_args) {
    unused(p_args);

    // @TODO: refacotr
    const char* module_name = "game_Debug.dll";
    LoadGameModule(module_name, m_module);

    std::shared_ptr<Scene> current_scene = m_app.GetSceneManager()->GetActiveScene();
    std::shared_ptr<Scene> sim_scene = std::make_shared<Scene>();
    sim_scene->Copy(*current_scene);
    sim_scene->Update(0.0f);

    m_app.GetSceneManager()->OpenSimScene(sim_scene);
    m_app.GetScriptManager()->OnSimBegin(*sim_scene);

    m_app.GetViewportManager()->CreateViewport(std::shared_ptr<ISceneViewProvider>(new RuntimeSceneViewProvider(m_app)));
}

void GameRuntimeState::OnExit() {
    m_app.GetViewportManager()->ClearViewport();

    m_app.GetScriptManager()->OnSimEnd();
    m_app.GetSceneManager()->CloseSimScene();

    UnloadGameModule(m_module);
}

void GameRuntimeState::Tick(float p_timestep) {
    // @TODO: tick game?

    if (ImguiManager* imgui_manager = m_app.GetImguiManager()) {
        imgui_manager->BeginFrame();

        if (BeginFullscreenWindow("Full Screen")) {
            const IGraphicsManager* gm = GetApp().GetGraphicsManager();
            uint64_t handle = gm->GetFinalImage();

            const Vector2i frame_size = DVAR_GET_IVEC2(resolution);
            ImGui::Image((ImTextureID)handle, ImVec2{ (float)frame_size.x, (float)frame_size.y });
        }
        EndFullscreenWindow();

        ImGui::Render();
    }

    if (std::shared_ptr<Scene> scene = m_app.GetSceneManager()->GetActiveScene()) {
        m_app.GetScriptManager()->Update(*scene, p_timestep);
    }

    // if (InputSystem::GetSingleton().IsActionJustPressed("ui_cancel")) {
    //     m_request = Some(StateRequest{ AppStateId::Editor });
    // }
}

Option<StateRequest> GameRuntimeState::PopRequest() {
    auto request = m_request;
    m_request = None();
    return request;
}

}  // namespace cave
