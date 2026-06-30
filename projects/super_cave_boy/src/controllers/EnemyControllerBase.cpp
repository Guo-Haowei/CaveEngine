#include "EnemyControllerBase.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;
using ::cave::ecs::Entity;

void EnemyControllerBase::onCreate(SceneContext& ctx) {
    player_ = findPlayer(ctx.query);
    animator_ = ctx.query.findChildByName("animator_node", entity());
}

Entity EnemyControllerBase::findPlayer(SceneQuery& query) const {
    return query.findFirstByName("player");
}

}  // namespace super_cave_boy
