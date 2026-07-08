#pragma once
#include "cave/game/IGameModule.h"
#include "cave/core/ids/Entity.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class ExitTrigger final : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

protected:
    void onBodyEntered(cave::SceneContext& ctx, Entity player) override;
    void onBodyExited(cave::SceneContext& ctx, Entity player) override;
};

}  // namespace super_cave_boy
