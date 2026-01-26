// =============================================================================
// File: engine/private/runtime/gameplay/GameRuntimeState.cpp
// =============================================================================
#include "GameRuntimeState.h"

#include <imgui/imgui.h>

#include "cave/runtime/gameplay/GameSession.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/framework/IGraphicsManager.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/framework/InputSystem.h"
#include "engine/private/runtime/framework/RuntimeHost.h"
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

// @TODO: refactor
class RuntimeSceneViewProvider : public ISceneViewProvider {
public:
    RuntimeSceneViewProvider(IApplication& p_app)
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
    IApplication& m_app;
};

GameRuntimeState::GameRuntimeState(IApplication& p_app)
    : AppState(p_app) {
    m_runtime_host = std::make_unique<RuntimeHost>(p_app);
}

GameRuntimeState::~GameRuntimeState() {
}

void GameRuntimeState::OnEnter(const StateRequest& p_args) {
    const char* module_name = "game_Debug.dll";
    LoadGameModule(module_name, m_module);

    if (m_module.api && m_module.api->RegisterGame) {
        GameLoadArgs args{};
        m_module.api->RegisterGame(m_app, args);
    }

    std::string_view mode = p_args.arg0;
    mode = "chess";  // @TODO: get correct game mode

    m_app.GetViewportManager()->CreateViewport(std::shared_ptr<ISceneViewProvider>(new RuntimeSceneViewProvider(m_app)));

    // @TODO: fix this part
    std::shared_ptr<Scene> current_scene = m_app.GetSceneManager()->GetActiveScene();
    RuntimeStartParams params(std::move(SceneSource::FromExisting(current_scene.get())));
    params.game_mode_id = mode;
    params.mode = RuntimeStartParams::Mode::PIE;
    m_runtime_host->Start(params);
}

void GameRuntimeState::OnExit() {
    m_app.GetViewportManager()->ClearViewport();

    m_runtime_host->Stop();

    UnloadGameModule(m_module);
}

void GameRuntimeState::Tick(float p_timestep) {
    GameFrameTime frame;
    frame.dt = p_timestep;
    m_runtime_host->Tick(frame);

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
}

Option<StateRequest> GameRuntimeState::PopRequest() {
    auto request = m_request;
    m_request = None();
    return request;
}

}  // namespace cave
