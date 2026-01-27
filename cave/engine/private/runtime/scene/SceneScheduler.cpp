#include "SceneScheduler.h"

#include "Scene.h"
#include "ISceneRegistry.h"

#include "engine/private/runtime/framework/IScriptManager.h"

namespace cave {

bool SceneScheduler::Register(ISceneTickContributor* p_contributor) {
    DEV_ASSERT(p_contributor);

    auto it = std::ranges::find(m_contributors, p_contributor);
    if (it != m_contributors.end()) {
        return false;
    }

    LOG_VERBOSE("SceneScheduler::Register: register scene contributor '{}'", (void*)p_contributor);
    m_contributors.push_back(p_contributor);
    return true;
}

bool SceneScheduler::Unregister(ISceneTickContributor* p_contributor) {
    DEV_ASSERT(p_contributor);

    auto it = std::ranges::find(m_contributors, p_contributor);
    if (it == m_contributors.end()) {
        return false;
    }

    m_contributors.erase(it);
    LOG_VERBOSE("SceneScheduler::Unregister: unregister scene contributor '{}'", (void*)p_contributor);
    return true;
}

void SceneScheduler::Tick(float p_dt) {
    std::vector<SceneTickRequest> requests;
    for (ISceneTickContributor* c : m_contributors) {
        if (c == nullptr) continue;
        c->CollectSceneTicks(requests);
    }

    // @TODO: merge same scenes from different contributors
    for (const SceneTickRequest& req : requests) {
        if (Scene* scene = m_scene_manager.Resolve(req.scene_id)) {
            if (req.mode == SceneTickMode::Simulation) {
                m_script_manager.Update(*scene, p_dt);
            }

            scene->Update(p_dt);
        }
    }
}

}  // namespace cave
