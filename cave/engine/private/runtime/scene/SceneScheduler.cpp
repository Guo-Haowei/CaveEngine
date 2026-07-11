#include "SceneScheduler.h"

#include "cave/core/time/FrameTime.h"
#include "cave/runtime/framework/EngineServices.h"

// @TODO: refactor
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

bool SceneScheduler::add(SceneOwner* owner) {
    DEV_ASSERT(owner);
    if (!owner) return false;

    auto it = std::ranges::find(m_owners, owner);
    if (it != m_owners.end()) return false;

    m_owners.push_back(owner);

#if USING(USE_LOG)
    const DebugId id = owner->debugId();
    LOG_TRACE(LogChannel::Scene, "+{}#{}", id.type, id.uid);
#endif
    return true;
}

bool SceneScheduler::remove(SceneOwner* owner) {
    DEV_ASSERT(owner);

    auto it = std::ranges::find(m_owners, owner);
    if (it == m_owners.end()) {
        return false;
    }

    m_owners.erase(it);

#if USING(USE_LOG)
    const DebugId id = owner->debugId();
    LOG_TRACE(LogChannel::Scene, "-{}#{}", id.type, id.uid);
#endif
    return true;
}

void SceneScheduler::flushSceneCommands() {
    for (SceneOwner* owner : m_owners) {
        if (owner) {
            owner->flushSceneCommands();
        }
    }
}

void SceneScheduler::tick(const FrameTime& time) {
    std::vector<SceneTickRequest> requests;
    for (SceneOwner* owner : m_owners) {
        if (owner) {
            owner->collectSceneTicks(requests);
        }
    }

    SceneRegistry& scene_registry = m_engine_services.sceneRegistry();
    for (const SceneTickRequest& req : requests) {
        if (Scene* scene = scene_registry.resolve(req.scene_id)) {
            SceneContext ctx(*scene, m_engine_services);
            ctx.view_id = req.view_id,
            ctx.scene_transition = &req.owner,

            scene->tick({
                .domain = req.mode,
                .dt = time.dt,
                .scene_ctx = ctx,
            });
        }
    }
}

}  // namespace cave
