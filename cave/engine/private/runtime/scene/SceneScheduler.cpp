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

    auto it = std::ranges::find(owners_, owner);
    if (it != owners_.end()) return false;

    owners_.push_back(owner);

#if USING(USE_LOG)
    const DebugId id = owner->debugId();
    LOG_TRACE(LogChannel::Scene, "+{}#{}", id.type, id.uid);
#endif
    return true;
}

bool SceneScheduler::remove(SceneOwner* owner) {
    DEV_ASSERT(owner);

    auto it = std::ranges::find(owners_, owner);
    if (it == owners_.end()) {
        return false;
    }

    owners_.erase(it);

#if USING(USE_LOG)
    const DebugId id = owner->debugId();
    LOG_TRACE(LogChannel::Scene, "-{}#{}", id.type, id.uid);
#endif
    return true;
}

void SceneScheduler::flushSceneCommands() {
    for (SceneOwner* owner : owners_) {
        if (owner) {
            owner->commitSceneChange();
            owner->commitSceneReload();
        }
    }
}

void SceneScheduler::tick(const FrameTime& time) {
    std::vector<SceneTickRequest> requests;
    for (SceneOwner* owner : owners_) {
        if (owner) {
            owner->collectSceneTicks(requests);
        }
    }

    SceneRegistry& scene_registry = services_.sceneRegistry();

    for (const SceneTickRequest& req : requests) {
        if (Scene* scene = scene_registry.resolve(req.scene_id)) {
            SceneContext ctx = {
                .native_scripts = services_.nativeScripts(),
                .scene = *scene,
                .scene_transition = req.owner,
                .query = SceneQuery(*scene),
                .engine_services = services_,
            };

            SceneTickContext tickCtx = {
                .mode = req.mode,
                .dt = time.dt,
                .scene_ctx = ctx,
            };
            scene->tick(tickCtx);
        }
    }
}

}  // namespace cave
