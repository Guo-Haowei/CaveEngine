// =============================================================================
// File: engine/public/cave/game/IGameModule.h
// =============================================================================
#pragma once
#include <cstdint>
#include "cave/core/time/FrameTime.h"

namespace cave {

enum class AppMode : uint8_t { Client,
                               Server,
                               Editor };

struct GameInitDesc {
    AppMode mode = AppMode::Client;
    const char* game_id = nullptr;
};

class IHostServices;
class World;

class IGameModule {
public:
    virtual ~IGameModule() = default;

    virtual void RegisterTypes(IHostServices& p_host) = 0;
    virtual void RegisterSystems(IHostServices& p_host) = 0;

    virtual void CreateWorld(World& world, IHostServices& p_host, const GameInitDesc& init) = 0;
    virtual void Tick(World& world, IHostServices& p_host, const FrameTime& time) = 0;

    virtual void ShutdownWorld(World& p_world, IHostServices& p_host) {
        (void)p_world;
        (void)p_host;
    }
};

}  // namespace cave

extern "C" {
__declspec(dllexport) cave::IGameModule* CreateGameModule();
}
