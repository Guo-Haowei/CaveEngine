#include "ChessGame.h"

#include "cave/core/diagnostics/ILogger.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandBuffer.h"

namespace cave {

void ChessGame::RegisterTypes(IHostServices& p_host) {
    unused(p_host);
}

void ChessGame::RegisterSystems(IHostServices& p_host) {
    unused(p_host);
}

void ChessGame::OnSceneBegin(Scene& p_scene, IHostServices& p_host, const GameInitDesc& p_init) {
    unused(p_init);

    p_host.Log().Print(LogLevel::LOG_LEVEL_OK, "hello from ChessGame\n");

    SceneCommandBuffer cb;
    ecs::Entity root = cb.Create();
    cb.Playback(p_scene);
}

void ChessGame::OnSceneEnd(Scene& p_scene, IHostServices& p_host) {
    unused(p_scene);
    unused(p_host);
}

void ChessGame::Tick(Scene& p_scene, IHostServices& p_host, const FrameTime& p_time) {
    unused(p_scene);
    unused(p_host);
    unused(p_time);
}

}  // namespace cave
