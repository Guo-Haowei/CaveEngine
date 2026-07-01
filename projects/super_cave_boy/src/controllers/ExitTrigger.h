#pragma once
#include "cave/game/IGameModule.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class ExitTrigger final : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

protected:
    void onTriggerEnter(cave::SceneContext& ctx, Entity player) override;
};

}  // namespace super_cave_boy
