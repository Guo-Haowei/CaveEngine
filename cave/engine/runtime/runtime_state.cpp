#include "runtime_state.h"

#include <imgui/imgui.h>

#include "engine/runtime/application.h"
#include "engine/runtime/graphics_manager_interface.h"
#include "engine/runtime/imgui_manager.h"
#include "engine/runtime/input_manager.h"
#include "engine/runtime/script_manager.h"
#include "engine/runtime/viewport_manager.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/scene/scene.h"
#include "engine/scene/scene_manager.h"

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
    m_app.GetScriptManager()->OnSimBegin(*sim_scene);

    m_app.GetViewportManager()->ClearViewport();
    m_app.GetViewportManager()->CreateViewport(std::shared_ptr<ISceneViewProvider>(new RuntimeSceneViewProvider(m_app)));
}

void RuntimeState::OnExit() {
    m_app.GetViewportManager()->ClearViewport();

    m_app.GetScriptManager()->OnSimEnd();
    m_app.GetSceneManager()->CloseSimScene();
}

void RuntimeState::Tick(float p_timestep) {
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

    CRASH_NOW();
#if 0
    if (InputManager::GetSingleton().IsActionJustPressed("ui_cancel")) {
        m_request = Some(StateRequest{ AppStateId::Editor });
    }
#endif
}

Option<StateRequest> RuntimeState::PopRequest() {
    auto request = m_request;
    m_request = None();
    return request;
}

}  // namespace cave
