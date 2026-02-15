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

struct GameInitDesc {
    AppMode mode = AppMode::Client;
    const char* game_id = nullptr;
};

class IHostServices;
class Scene;
class SceneCommandBuffer;

class IGameModule {
public:
    virtual ~IGameModule() = default;

    virtual void RegisterTypes(IHostServices& p_host) = 0;
    virtual void RegisterSystems(IHostServices& p_host) = 0;

    virtual void OnSceneBegin(Scene& p_scene,
                              IHostServices& p_host,
                              const GameInitDesc& p_init,
                              SceneCommandBuffer& p_cb) = 0;
    virtual void OnSceneEnd(Scene& p_scene, IHostServices& p_host) = 0;

    virtual void Tick(Scene& p_scene, IHostServices& p_host, const FrameTime& p_time) = 0;
};

}  // namespace cave
