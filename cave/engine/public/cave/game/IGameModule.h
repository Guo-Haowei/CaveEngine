// =============================================================================
// File: engine/public/cave/game/IGameModule.h
// =============================================================================
#pragma once
#include <cstdint>
#include "cave/core/time/FrameTime.h"

namespace cave {

enum class AppMode : uint8_t {
    Client,
    Server,
    Editor,
};

class IHostServices;

class IGameModule {
public:
    virtual ~IGameModule() = default;

    virtual void OnModuleLoaded(IHostServices& p_host) = 0;
    virtual void OnModuleUnloaded(IHostServices& p_host) = 0;

    virtual void OnGameBegin(IHostServices& p_host) = 0;
    virtual void OnGameEnd(IHostServices& p_host) = 0;

    virtual void Tick(IHostServices& p_host, const FrameTime& p_time) = 0;
};

}  // namespace cave
