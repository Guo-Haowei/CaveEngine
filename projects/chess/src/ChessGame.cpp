#include "ChessGame.h"

#include "cave/core/diagnostics/ILogger.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneMutatorExt.h"

namespace cave {

void ChessGame::RegisterTypes(IHostServices& p_host) {
    unused(p_host);
}

void ChessGame::RegisterSystems(IHostServices& p_host) {
    unused(p_host);
}

void ChessGame::OnSceneBegin(Scene& p_scene,
                             IHostServices& p_host,
                             const GameInitDesc& p_init,
                             SceneCommandBuffer& p_cb) {
    unused(p_init);
    unused(p_scene);

    p_host.Log().Print(LogLevel::LOG_LEVEL_OK, "hello from ChessGame\n");

    SceneExt scene_ext(p_host.AssetRegistry());
    ecs::Entity cube = scene_ext.CreateCubeObject(p_cb, "my-light");
    p_cb.AttachRoot(cube);
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
