#include "RuntimeHost.h"

#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/gameplay/GameSession.h"

#include "engine/private/runtime/scene/SceneManager.h"
#include "engine/private/runtime/framework/ScriptManager.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

RuntimeHost::RuntimeHost(IApplication& p_app)
    : m_app(p_app) {}

RuntimeHost::~RuntimeHost() {}

void RuntimeHost::Start(const RuntimeStartParams& p_params) {
    SceneManager& scene_manager = *m_app.GetSceneManager();

    switch (p_params.source.type) {
        case SceneSource::Type::FromPath: {
            CRASH_NOW_MSG("TODO");
        } break;
        case SceneSource::Type::FromExisting: {
            m_scene_id = scene_manager.Clone({ "" }, p_params.source.existing);
        } break;
    }

    Scene* scene = scene_manager.Resolve(m_scene_id);
    DEV_ASSERT(scene);
    m_app.GetScriptManager()->OnSimBegin(*scene);
    m_session = std::make_unique<GameSession>(m_app.GetGameModeFactory());
    m_session->Start(p_params.game_mode_id);
}

void RuntimeHost::Stop() {
    m_session->Stop();
    m_session.reset();

    if (Scene* scene = m_app.GetSceneManager()->Resolve(m_scene_id)) {
        m_app.GetScriptManager()->OnSimEnd();
    }
}

void RuntimeHost::Tick(const GameFrameTime& p_frame) {
    if (m_session) {
        m_session->Tick(p_frame);
    }

    if (Scene* scene = m_app.GetSceneManager()->Resolve(m_scene_id)) {
        m_app.GetScriptManager()->Update(*scene, p_frame.dt);
        scene->Update(p_frame.dt);
    }
}

}  // namespace cave
