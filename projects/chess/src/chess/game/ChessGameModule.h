#pragma once
#include <memory>
#include "cave/core/typedefs.h"
#include "cave/game/IGameModule.h"

namespace chess {

class ChessGameMode;

class ChessGameModule final : public cave::IGameModule {
public:
    ChessGameModule();
    ~ChessGameModule();

    void onModuleLoaded(cave::IHostServices& host) override;
    void onModuleUnloaded(cave::IHostServices& host) override;

    // @TODO: move these to ChessGameMode,
    // ChessGameModule should only be responsible for DLL loading
    void onGameBegin(cave::IHostServices& host) override;
    void onGameEnd(cave::IHostServices& host) override;

    void tick(cave::IHostServices& host, const cave::FrameTime& time) override;

private:
    void spawnObjects(cave::IHostServices& host);

    std::unique_ptr<ChessGameMode> game_;
};

}  // namespace chess

extern "C" {

CAVE_API ::cave::IGameModule* CreateGameModule() {
    return new ::chess::ChessGameModule();
}

CAVE_API void DestroyGameModule(::cave::IGameModule* game) {
    if (game != nullptr) {
        delete game;
    }
}
}
