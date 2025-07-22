#include "layer.h"

#include "engine/runtime/application.h"
#include "engine/runtime/physics_manager.h"
#include "engine/runtime/scene_manager_interface.h"
#include "engine/runtime/script_manager.h"

namespace cave {

void GameLayer::SetActiveScene(std::shared_ptr<Scene>&& p_scene) {
    m_scene = std::move(p_scene);
}

void GameLayer::OnAttach() {
    LOG("GameLayer '{}' attached", m_name);

    OnAttachInternal();

    if (DEV_VERIFY(m_scene)) {
        m_app->GetScriptManager()->OnSimBegin(*m_scene);
        m_app->GetPhysicsManager()->OnSimBegin(*m_scene);
    }
}

void GameLayer::OnDetach() {
    LOG("GameLayer '{}' detached", m_name);

    m_app->GetPhysicsManager()->OnSimEnd();
    m_app->GetScriptManager()->OnSimEnd();

    OnDetachInternal();
}

}  // namespace cave
