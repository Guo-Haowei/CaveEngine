#include "SceneScheduler.h"

#include "cave/core/time/FrameTime.h"
#include "cave/runtime/framework/EngineServices.h"

// @TODO: refactor
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

bool SceneScheduler::add(ISceneTickContributor* contributor) {
    DEV_ASSERT(contributor);
    if (!contributor) return false;

    auto it = std::ranges::find(contributors_, contributor);
    if (it != contributors_.end()) return false;

    contributors_.push_back(contributor);

#if USING(USE_LOG)
    const DebugId id = contributor->debugId();
    LOG_TRACE(LogChannel::Scene, "+{}#{}", id.type, id.uid);
#endif
    return true;
}

bool SceneScheduler::remove(ISceneTickContributor* contributor) {
    DEV_ASSERT(contributor);

    auto it = std::ranges::find(contributors_, contributor);
    if (it == contributors_.end()) {
        return false;
    }

    contributors_.erase(it);

#if USING(USE_LOG)
    const DebugId id = contributor->debugId();
    LOG_TRACE(LogChannel::Scene, "-{}#{}", id.type, id.uid);
#endif
    return true;
}

void SceneScheduler::tick(const FrameTime& time) {
    std::vector<SceneTickRequest> requests;
    for (ISceneTickContributor* c : contributors_) {
        if (c == nullptr) continue;
        c->collectSceneTicks(requests);
    }

    SceneRegistry& scene_registry = services_.sceneRegistry();

    //// @TODO: merge same scenes from different contributors
    for (const SceneTickRequest& req : requests) {
        if (Scene* scene = scene_registry.resolve(req.scene_id)) {
            SceneContext ctx = {
                .native_scripts = services_.nativeScripts(),
                .scene = *scene,
                .query = SceneQuery(*scene),
                .engine_services = services_,
            };

            SceneTickContext tickCtx = {
                .mode = req.mode,
                .dt = time.dt,
                .sceneCtx = ctx,
            };
            scene->tick(tickCtx);
        }
    }
}

}  // namespace cave
