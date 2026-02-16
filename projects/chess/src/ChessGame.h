#pragma once
#include "cave/core/typedefs.h"
#include "cave/game/IGameModule.h"

namespace cave {

class ChessGame final : public IGameModule {
public:
    void OnModuleLoaded(IHostServices& p_host) override;

    void Tick(IHostServices& p_host, const FrameTime& p_time) override;

private:
    void SpawnPieces(IHostServices& p_host);
};

}  // namespace cave

extern "C" {
CAVE_API cave::IGameModule* CreateGameModule() {
    return new ::cave::ChessGame();
}
}
