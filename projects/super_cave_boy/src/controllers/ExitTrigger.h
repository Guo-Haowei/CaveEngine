#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/runtime/game/IGameModule.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class ExitTrigger final : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

protected:
    void onBodyEntered(Entity player) override;
    void onBodyExited(Entity player) override;
};

}  // namespace super_cave_boy
