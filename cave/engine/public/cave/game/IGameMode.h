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

    virtual void OnEnter(IHostServices& p_host) = 0;
    virtual void OnExit(IHostServices& p_host) = 0;
    virtual void Tick(IHostServices& p_host, const FrameTime& p_time) = 0;
};

}  // namespace cave
