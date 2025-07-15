#include "layer.h"

#include "engine/runtime/application.h"
#include "engine/runtime/physics_manager.h"
#include "engine/runtime/scene_manager_interface.h"
#include "engine/runtime/script_manager.h"

namespace cave {

void GameLayer::OnAttach() {
    LOG("GameLayer '{}' attached", m_name);

    OnAttachInternal();

    Scene* scene = m_app->GetSceneManager()->GetActiveScene();
    if (DEV_VERIFY(scene)) {
        m_app->GetScriptManager()->OnSimBegin(*scene);
        m_app->GetPhysicsManager()->OnSimBegin(*scene);
    }
}

void GameLayer::OnDetach() {
    LOG("GameLayer '{}' detached", m_name);

    Scene* scene = m_app->GetSceneManager()->GetActiveScene();
    if (DEV_VERIFY(scene)) {
        m_app->GetPhysicsManager()->OnSimEnd(*scene);
        m_app->GetScriptManager()->OnSimEnd(*scene);
    }

    OnDetachInternal();
}

}  // namespace cave
