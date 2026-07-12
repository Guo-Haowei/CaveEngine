#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/math/Vec.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class EnemyControllerBase : public cave::NativeScript {
    using Entity = cave::ecs::Entity;

protected:
    void start() override;
    void destroy() override;

    void onBodyEntered(Entity player) override;
    void onBodyStay(Entity player) override;

    Entity findPlayer() const;
    void playAnimation(std::string_view name);

    Entity m_player{};
    Entity m_animator{};

    int m_health = 1;
};

}  // namespace super_cave_boy
