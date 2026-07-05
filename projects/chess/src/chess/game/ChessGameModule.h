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

    void registerNativeScripts(cave::NativeScriptRegistry& registry) override;

    void onGameBegin(cave::IHostServices& host) override;
    void onGameEnd(cave::IHostServices& host) override;

    void tick(cave::IHostServices& host, const cave::FrameTime& time) override;
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
