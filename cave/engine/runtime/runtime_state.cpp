#include "runtime_state.h"

#include <imgui/imgui.h>

#include "engine/runtime/application.h"
#include "engine/runtime/graphics_manager_interface.h"
#include "engine/runtime/imgui_manager.h"
#include "engine/runtime/script_manager.h"
#include "engine/runtime/viewport_manager.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/scene/scene.h"
#include "engine/scene/scene_manager.h"

namespace cave {

class RuntimeSceneViewProvider : public ISceneViewProvider {
public:
    RuntimeSceneViewProvider(Application& p_app)
        : m_app(p_app) {}

    void Update(float,
                const ViewportInput&,
                bool) final {}

    void BuildViews(std::vector<SceneView>& p_out_views,
                    bool p_is_opengl) final {
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

    // set active scene?
    // game_layer->SetActiveScene(std::move(sim_scene));
    m_app.GetScriptManager()->OnSimBegin(*sim_scene);

    // reset viewport
    m_app.GetViewportManager()->ClearViewport();
    m_app.GetViewportManager()->CreateViewport(std::shared_ptr<ISceneViewProvider>(new RuntimeSceneViewProvider(m_app)));
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

            const Vector2i frame_size = DVAR_GET_IVEC2(resolution);
            ImGui::Image((ImTextureID)handle, ImVec2{ (float)frame_size.x, (float)frame_size.y });
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
