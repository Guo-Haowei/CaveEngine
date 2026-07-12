#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/runtime/game/IGameModule.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class CameraController final : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

public:
    void start() override;

    void update(float dt) override;

private:
    void followTarget(float dt);

    Entity m_target;
};

}  // namespace super_cave_boy
