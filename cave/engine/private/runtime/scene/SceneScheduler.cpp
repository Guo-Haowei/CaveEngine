#include "SceneScheduler.h"

#include "Scene.h"
#include "SceneManager.h"

#include "engine/private/runtime/framework/IScriptManager.h"

namespace cave {

void SceneScheduler::Add(const SceneTickRequest& p_request) {
    m_requests.emplace_back(p_request);
}

void SceneScheduler::Tick(float p_dt) {
    for (const SceneTickRequest& req : m_requests) {
        Scene* scene = m_scene_manager.Resolve(req.scene_id);
        if (scene) {
            if (req.mode == SceneTickMode::Simulation) {
                m_script_manager.Update(*scene, p_dt);
            }

            scene->Update(p_dt);
        }
    }
    m_requests.clear();
}

}  // namespace cave
