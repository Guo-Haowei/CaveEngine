#include "RuntimeHost.h"

#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/gameplay/GameSession.h"

#include "engine/private/runtime/framework/ISceneManager.h"
#include "engine/private/runtime/framework/ScriptManager.h"
#include "engine/private/scene/scene.h"

namespace cave {

RuntimeHost::RuntimeHost(IApplication& p_app)
    : m_app(p_app) {}

void RuntimeHost::Start(const RuntimeStartParams& p_params) {
    std::shared_ptr<Scene> sim_scene = std::make_shared<Scene>();
    switch (p_params.source.type) {
        case SceneSource::Type::FromPath:
            //m_world = WorldLoader::Load(p_params.source.path);
            break;
        case SceneSource::Type::FromExisting: {
            sim_scene->Copy(*p_params.source.existing);
            sim_scene->Update(0.0f);
        } break;
    }

    m_app.GetSceneManager()->OpenSimScene(sim_scene);
    m_app.GetScriptManager()->OnSimBegin(*sim_scene);

    m_session = std::make_unique<GameSession>(m_app.GetGameModeFactory());

    //m_session->SetMode(p_params.mode);
    m_session->Start(p_params.game_mode_id);
}

void RuntimeHost::Stop() {
    m_session->Stop();
    m_session.reset();

    m_app.GetScriptManager()->OnSimEnd();
    m_app.GetSceneManager()->CloseSimScene();
}

void RuntimeHost::Tick(const GameFrameTime& p_frame) {
    if (m_session) {
        m_session->Tick(p_frame);
    }

    if (std::shared_ptr<Scene> scene = m_app.GetSceneManager()->GetActiveScene()) {
        m_app.GetScriptManager()->Update(*scene, p_frame.dt);
    }
}

#if 0
void EditorState::EnterPlayMode() {
    // 1) Clone the current editor scene
    m_play_world = m_editor_world->Clone();

    // 2) Freeze editor mutations
    m_editor_world->SetReadOnly(true);

    // 3) Build runtime params
    RuntimeStartParams params;
    params.mode = RuntimeStartParams::Mode::PlayInEditor;
    params.source = WorldSource::FromExisting(m_play_world.get());
    params.game_mode_id = "chess";
    params.input_provider = &m_pie_input_router;
    params.enable_rendering = true;
    params.enable_pause = true;

    // 4) Start runtime
    m_runtime_host.Start(params);

    m_is_playing = true;
}

void EditorState::ExitPlayMode() {
    // 1) Stop runtime
    m_runtime_host.Stop();

    // 2) Destroy play world
    m_play_world.reset();

    // 3) Unfreeze editor scene
    m_editor_world->SetReadOnly(false);

    m_is_playing = false;
}
#endif

}  // namespace cave
