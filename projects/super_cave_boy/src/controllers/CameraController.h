#pragma once
#include "cave/game/IGameModule.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class CameraController final : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

public:
    void onCreate(cave::SceneContext& ctx) override;
    void onDestroy(cave::SceneContext& ctx) override;

    void onUpdate(cave::SceneContext& ctx, float dt) override;

private:
    void followTarget(cave::SceneContext& ctx, float dt);

    Entity target_;
};

}  // namespace super_cave_boy
