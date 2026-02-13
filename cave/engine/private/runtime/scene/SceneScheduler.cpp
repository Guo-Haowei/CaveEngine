#include "SceneScheduler.h"

#include "cave/core/time/FrameTime.h"

#include "engine/private/runtime/framework/IScriptService.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

bool SceneScheduler::Register(ISceneTickContributor* p_contributor) {
    DEV_ASSERT(p_contributor);
    if (!p_contributor) return false;

    auto it = std::ranges::find(m_contributors, p_contributor);
    if (it != m_contributors.end()) return false;

    m_contributors.push_back(p_contributor);

#if USING(USE_LOG)
    DebugId id = p_contributor->GetDebugId();
    LOG_VERBOSE("SceneScheduler::Register: register scene contributor '{}(id:{})'", id.type, id.uid);
#endif
    return true;
}

bool SceneScheduler::Unregister(ISceneTickContributor* p_contributor) {
    DEV_ASSERT(p_contributor);

    auto it = std::ranges::find(m_contributors, p_contributor);
    if (it == m_contributors.end()) {
        return false;
    }

    m_contributors.erase(it);

#if USING(USE_LOG)
    DebugId id = p_contributor->GetDebugId();
    LOG_VERBOSE("SceneScheduler::Unregister: unregister scene contributor '{}(id:{})'", id.type, id.uid);
#endif
    return true;
}

void SceneScheduler::Tick(const FrameTime& p_time) {
    std::vector<SceneTickRequest> requests;
    for (ISceneTickContributor* c : m_contributors) {
        if (c == nullptr) continue;
        c->CollectSceneTicks(requests);
    }

    // @TODO: merge same scenes from different contributors
    for (const SceneTickRequest& req : requests) {
        if (Scene* scene = m_scene_manager.Resolve(req.scene_id)) {
            if (req.mode == SceneTickMode::Simulation) {
                m_script_manager.Update(*scene, p_time.dt);
            }

            scene->Update(p_time.dt);
        }
    }
}

}  // namespace cave
