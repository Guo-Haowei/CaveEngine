// =============================================================================
// File: cave/game/IGameMode.h
// =============================================================================
#pragma once
#include "cave/core/time/FrameTime.h"

namespace cave {

class IHostServices;

class IGameMode {
public:
    virtual ~IGameMode() = default;

    virtual void onEnter(IHostServices& host) = 0;
    virtual void onExit(IHostServices& host) = 0;
    virtual void tick(IHostServices& host, const FrameTime& time) = 0;
};

}  // namespace cave
