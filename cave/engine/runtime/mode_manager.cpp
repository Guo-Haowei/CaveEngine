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

}  // namespace cave
