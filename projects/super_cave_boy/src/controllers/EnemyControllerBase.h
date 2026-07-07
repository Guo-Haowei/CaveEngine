#pragma once
#include "cave/core/math/Vector.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class EnemyControllerBase : public cave::NativeScript {
    using Entity = cave::ecs::Entity;

protected:
    void start(cave::SceneContext& ctx) override;
    void destroy() override;

    void onBodyEntered(cave::SceneContext& ctx, Entity player) override;
    void onBodyStay(cave::SceneContext& ctx, Entity player) override;

    Entity findPlayer(cave::SceneQuery& query) const;

    Entity m_player{};
    Entity m_animator{};
};

}  // namespace super_cave_boy
