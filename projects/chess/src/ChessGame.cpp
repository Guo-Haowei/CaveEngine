#include "ChessGame.h"

#include "cave/core/diagnostics/ILogger.h"
#include "cave/game/IHostServices.h"

namespace cave {

void ChessGame::RegisterTypes(IHostServices& p_host) {
    (void)p_host;
}

void ChessGame::RegisterSystems(IHostServices& p_host) {
    p_host.Log().Print(LogLevel::LOG_LEVEL_OK, "hello from ChessGame\n");
}

void ChessGame::CreateWorld(World& world, IHostServices& p_host, const GameInitDesc& init) {
    (void)p_host;
    (void)world;
    (void)init;
}

void ChessGame::Tick(World& world, IHostServices& p_host, const FrameTime& time) {
    (void)p_host;
    (void)world;
    (void)time;
}

}  // namespace cave
