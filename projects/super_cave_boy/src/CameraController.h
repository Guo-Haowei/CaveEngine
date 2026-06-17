#pragma once
#include "cave/game/IGameModule.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class CameraController : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

public:
    void onCreate(cave::IHostServices& host);
    void onDestroy(cave::IHostServices& host);

    void onUpdate(cave::IHostServices& host, const cave::FrameTime& time);

private:
    void followTarget(cave::IHostServices& host, float dt);

    Entity camera_;
    Entity target_;
};

}  // namespace super_cave_boy
