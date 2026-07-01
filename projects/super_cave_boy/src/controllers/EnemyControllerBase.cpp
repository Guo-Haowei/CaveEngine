#include "EnemyControllerBase.h"

#include "cave/core/diagnostics/Log.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;
using ::cave::ecs::Entity;

void EnemyControllerBase::onCreate(SceneContext& ctx) {
    player_ = findPlayer(ctx.query);
    animator_ = ctx.query.findChildByName("animator_node", entity());
}

void EnemyControllerBase::onDestroy() {
    LOG_INFO(LogChannel::Game, "entity destroyed");
}

Entity EnemyControllerBase::findPlayer(SceneQuery& query) const {
    return query.findFirstByName("player");
}

}  // namespace super_cave_boy
