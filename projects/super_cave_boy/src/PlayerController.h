#pragma once
#include "cave/game/IGameModule.h"

namespace super_cave_boy {

class PlayerController {
public:
    void onCreate(cave::IHostServices& host);
    void onDestroy(cave::IHostServices& host);

    void onUpdate(cave::IHostServices& host, const cave::FrameTime& time);
};

}  // namespace super_cave_boy
