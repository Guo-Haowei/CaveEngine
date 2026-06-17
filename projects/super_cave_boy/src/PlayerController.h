#pragma once
#include "cave/game/IGameModule.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class PlayerController : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

public:
    void onCreate() override;
    void onDestroy() override;

    void onUpdate(float dt) override;

private:
    Entity animator_;
};

}  // namespace super_cave_boy
