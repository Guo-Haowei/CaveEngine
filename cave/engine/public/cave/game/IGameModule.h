// =============================================================================
// File: cave/game/IGameModule.h
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

    virtual void onModuleLoaded(IHostServices& host) = 0;
    virtual void onModuleUnloaded(IHostServices& host) = 0;

    virtual void onGameBegin(IHostServices& host) = 0;
    virtual void onGameEnd(IHostServices& host) = 0;

    virtual void tick(IHostServices& host, const FrameTime& time) = 0;
};

}  // namespace cave
