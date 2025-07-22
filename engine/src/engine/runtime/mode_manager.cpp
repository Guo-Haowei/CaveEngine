#include "mode_manager.h"

#include "engine/runtime/application.h"
#include "engine/runtime/scene_manager_interface.h"
#include "engine/runtime/script_manager.h"
#include "engine/scene/scene.h"

namespace cave {
#if 0
    switch (m_app->GetState()) {
        case Application::State::EDITING: {
            m_app->SetActiveScene(scene);
        } break;
        case Application::State::BEGIN_SIM: {
        } break;
        case Application::State::END_SIM: {
            m_app->DetachGameLayer();

            m_app->SetActiveScene(scene);
            m_app->SetState(Application::State::EDITING);

            if (DEV_VERIFY(m_simScene)) {
                delete m_simScene;
                m_simScene = nullptr;
            }
        } break;
    }
#endif

void ModeManager::SetMode(GameMode p_mode) {
    LOG("attempt to transit from mode {} to {}", (int)m_mode, (int)p_mode);
    if (p_mode == m_mode) {
        return;
    }

    switch (p_mode) {
        case GameMode::Editor: {
            LOG_ERROR("not implemented");
        } break;
        case GameMode::Gameplay: {
            Scene* current_scene = ISceneManager::GetSingleton().GetActiveScene();
            Scene* sim_scene = new Scene;
            sim_scene->Copy(*current_scene);
            sim_scene->Update(0.0f);

            ISceneManager::GetSingleton().SetActiveScene(sim_scene);

            m_app.AttachGameLayer();
        } break;
        case GameMode::CutScene:
        case GameMode::Loading:
        case GameMode::Paused:
        default:
            CRASH_NOW_MSG("mode not supported");
            break;
    }

    m_mode = p_mode;
}

}  // namespace cave
