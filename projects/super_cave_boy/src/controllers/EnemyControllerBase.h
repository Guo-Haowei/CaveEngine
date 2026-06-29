#pragma once
#include "cave/core/math/Vector.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class EnemyControllerBase : public cave::NativeScript {
protected:
    void onCreate() override;

    void onCollision(cave::ecs::Entity ent) override;

    cave::ecs::Entity findPlayer(cave::SceneQuery& query) const;

    cave::ecs::Entity player_{};
    cave::ecs::Entity animator_{};
};

}  // namespace super_cave_boy
