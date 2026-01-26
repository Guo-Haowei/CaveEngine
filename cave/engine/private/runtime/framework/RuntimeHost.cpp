#include "RuntimeHost.h"

#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/gameplay/GameSession.h"

#include "engine/private/runtime/scene/ISceneManager.h"
#include "engine/private/runtime/framework/ScriptManager.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

RuntimeHost::RuntimeHost(IApplication& p_app)
    : m_app(p_app) {}

RuntimeHost::~RuntimeHost() {}

void RuntimeHost::Start(const RuntimeStartParams& p_params) {
    std::shared_ptr<Scene> sim_scene = std::make_shared<Scene>();
    switch (p_params.source.type) {
        case SceneSource::Type::FromPath: {
            CRASH_NOW_MSG("TODO");
        } break;
        case SceneSource::Type::FromExisting: {
            sim_scene->Copy(*p_params.source.existing);
            sim_scene->Update(0.0f);
        } break;
    }

    DEV_ASSERT(0);
    // m_app.GetSceneManager()->OpenSimScene(sim_scene);
    // m_app.GetScriptManager()->OnSimBegin(*sim_scene);

    m_session = std::make_unique<GameSession>(m_app.GetGameModeFactory());

    m_session->Start(p_params.game_mode_id);
}

void RuntimeHost::Stop() {
    m_session->Stop();
    m_session.reset();

    DEV_ASSERT(0);
    // m_app.GetScriptManager()->OnSimEnd();
    // m_app.GetSceneManager()->CloseSimScene();
}

void RuntimeHost::Tick(const GameFrameTime& p_frame) {
    if (m_session) {
        m_session->Tick(p_frame);
    }

    DEV_ASSERT(0);
    // if (std::shared_ptr<Scene> scene = m_app.GetSceneManager()->GetActiveScene()) {
    //     m_app.GetScriptManager()->Update(*scene, p_frame.dt);
    // }
}

}  // namespace cave
