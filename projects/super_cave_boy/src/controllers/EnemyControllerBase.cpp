#include "EnemyControllerBase.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;
using ::cave::ecs::Entity;

void EnemyControllerBase::onCreate() {
    SceneQuery query(context().scene);

    player_ = findPlayer(query);
    animator_ = query.findChildByName("animator_node", entity());
}

Entity EnemyControllerBase::findPlayer(SceneQuery& query) const {
    return query.findFirstByName("player");
}

}  // namespace super_cave_boy
