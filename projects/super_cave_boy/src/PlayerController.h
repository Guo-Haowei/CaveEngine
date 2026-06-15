#pragma once
#include "cave/game/IGameModule.h"
#include "cave/runtime/ecs/Entity.h"

namespace super_cave_boy {

class PlayerController {
    using Entity = cave::ecs::Entity;
public:
    void onCreate(cave::IHostServices& host);
    void onDestroy(cave::IHostServices& host);

    void onUpdate(cave::IHostServices& host, const cave::FrameTime& time);

private:
    void initLevel(cave::IHostServices& host);

    Entity player_;
    Entity player_animator_;
};

}  // namespace super_cave_boy
