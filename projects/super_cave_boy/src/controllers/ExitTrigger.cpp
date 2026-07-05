#include "ExitTrigger.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/runtime/scene/ISceneTransitionRequests.h"
#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/scene/SceneQuery.h"

#include "Utility.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;

void ExitTrigger::onBodyEntered(SceneContext& ctx, Entity player) {
    SceneQuery& query = ctx.query;
#if USING(ENABLE_ASSERT)
    auto* player_collider = query.component<ColliderComponent>(player);
    DEV_ASSERT(player_collider);
    DEV_ASSERT(IsPlayer(*player_collider));
#endif

    auto it = params().find("level");
    if (it != params().end()) {
        std::string_view level = it->second.asString();
        ctx.scene_transition.requestSceneChange(std::format("@res://scenes/{}.scene", level));
    }
}

void ExitTrigger::onBodyExited(SceneContext& ctx, Entity player) {
    unused(ctx);
    unused(player);

    LOG_OK("Exited portal!");
}

}  // namespace super_cave_boy
