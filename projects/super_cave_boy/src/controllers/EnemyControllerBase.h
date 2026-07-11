#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/math/Vec.h"
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
    void playAnimation(cave::SceneContext& ctx, std::string_view name);

    Entity m_player{};
    Entity m_animator{};
};

}  // namespace super_cave_boy
