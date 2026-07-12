// =============================================================================
// File: cave/runtime/game/IGameMode.h
// =============================================================================
#pragma once
#include "cave/core/time/FrameTime.h"

namespace cave {

class IGameMode {
public:
    virtual ~IGameMode() = default;

    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void tick(float dt) = 0;
};

}  // namespace cave
