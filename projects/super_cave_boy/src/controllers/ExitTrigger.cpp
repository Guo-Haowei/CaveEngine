#include "ExitTrigger.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/scene/SceneQuery.h"

#include "Utility.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;

void ExitTrigger::onTriggerEnter(cave::SceneContext& ctx, Entity player) {
    SceneQuery& query = ctx.query;
#if USING(ENABLE_ASSERT)
    auto* player_collider = query.component<ColliderComponent>(player);
    DEV_ASSERT(player_collider);
    DEV_ASSERT(IsPlayer(*player_collider));
#endif

    LOG_OK("Enter portal!");
}

}  // namespace super_cave_boy
