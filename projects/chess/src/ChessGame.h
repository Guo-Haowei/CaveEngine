#pragma once
#include "cave/core/typedefs.h"
#include "cave/game/IGameModule.h"

namespace cave {

class ChessGame final : public IGameModule {
public:
    void RegisterTypes(IHostServices& p_host) override;
    void RegisterSystems(IHostServices& p_host) override;

    void OnSceneBegin(Scene& p_scene,
                      IHostServices& p_host,
                      const GameInitDesc& p_init,
                      SceneCommandBuffer& p_cb) override;

    void OnSceneEnd(Scene& p_scene, IHostServices& p_host) override;

    void Tick(Scene& p_scene, IHostServices& p_host, const FrameTime& p_time) override;

private:
    void CreatePieces(IHostServices& p_host, SceneCommandBuffer& p_cb);
};

}  // namespace cave

extern "C" {
CAVE_API cave::IGameModule* CreateGameModule() {
    return new ::cave::ChessGame();
}
}
