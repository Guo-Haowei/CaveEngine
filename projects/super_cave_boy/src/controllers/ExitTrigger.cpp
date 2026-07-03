#include "ExitTrigger.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/runtime/scene/ISceneTransitionRequests.h"
#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/scene/SceneQuery.h"

#include "Utility.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;

void ExitTrigger::onBodyEntered(cave::SceneContext& ctx, Entity player) {
    SceneQuery& query = ctx.query;
#if USING(ENABLE_ASSERT)
    auto* player_collider = query.component<ColliderComponent>(player);
    DEV_ASSERT(player_collider);
    DEV_ASSERT(IsPlayer(*player_collider));
#endif

    ctx.scene_transition.requestSceneChange("@res://scenes/level_10.scene");
}

void ExitTrigger::onBodyExited(cave::SceneContext& ctx, Entity player) {
    unused(ctx);
    unused(player);

    LOG_OK("Exited portal!");
}

}  // namespace super_cave_boy
