#pragma once
#include <memory>
#include "cave/core/typedefs.h"
#include "cave/game/IGameModule.h"

class GameModule final : public cave::IGameModule {
public:
    GameModule();
    ~GameModule();

    void onModuleLoaded(cave::IHostServices& host) override;
    void onModuleUnloaded(cave::IHostServices& host) override;

    void onGameBegin(cave::IHostServices& host) override;
    void onGameEnd(cave::IHostServices& host) override;

    void tick(cave::IHostServices& host, const cave::FrameTime& time) override;

private:
};

extern "C" {
CAVE_API cave::IGameModule* CreateGameModule() {
    return new ::GameModule();
}
}
