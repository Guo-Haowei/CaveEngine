#include "SceneScheduler.h"

#include "cave/core/time/FrameTime.h"

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

    //// @TODO: merge same scenes from different contributors
    for (const SceneTickRequest& req : requests) {
        if (Scene* scene = scene_manager_.resolve(req.scene_id)) {
            // @TODO: this should be ticked inside scene::Update()
            if (req.mode == SceneTickMode::Simulation) {
                scene->simulate(time.dt);
            }

            scene->tick(time.dt);
        }
    }
}

}  // namespace cave
