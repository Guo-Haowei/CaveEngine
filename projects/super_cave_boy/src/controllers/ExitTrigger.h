#pragma once
#include "cave/game/IGameModule.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class ExitTrigger final : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

public:
    void onCreate(cave::SceneContext& ctx) override;
};

}  // namespace super_cave_boy
