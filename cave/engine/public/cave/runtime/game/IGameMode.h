// =============================================================================
// File: cave/runtime/game/IGameMode.h
// =============================================================================
#pragma once
#include "cave/core/time/FrameTime.h"

namespace cave {

struct SceneContext;

class IGameMode {
public:
    virtual ~IGameMode() = default;

    virtual void onEnter(SceneContext& host) = 0;
    virtual void onExit() = 0;
    virtual void tick(SceneContext& host, float dt) = 0;
};

}  // namespace cave
