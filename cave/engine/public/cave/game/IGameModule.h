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
class Scene;
class SceneCommandWriter;

class IGameModule {
public:
    virtual ~IGameModule() = default;

    virtual void OnModuleLoaded(IHostServices&,
                                Scene&,
                                SceneCommandWriter&) {}

    virtual void Tick(IHostServices& p_host,
                      const FrameTime& p_time) = 0;
};

}  // namespace cave
