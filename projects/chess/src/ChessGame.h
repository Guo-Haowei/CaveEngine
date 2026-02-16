#pragma once
#include <memory>
#include "cave/core/typedefs.h"
#include "cave/game/IGameModule.h"
#include "cave/runtime/controller/GridSelectController.h"

namespace cave {

class ChessGame final : public IGameModule {
public:
    void OnModuleLoaded(IHostServices& p_host) override;
    void OnModuleUnloaded(IHostServices& p_host) override;

    void OnGameBegin(IHostServices& p_host) override;
    void OnGameEnd(IHostServices& p_host) override;

    void Tick(IHostServices& p_host, const FrameTime& p_time) override;

private:
    void SpawnPieces(IHostServices& p_host);

    std::unique_ptr<GridSelectController> m_selector;
};

}  // namespace cave

extern "C" {
CAVE_API cave::IGameModule* CreateGameModule() {
    return new ::cave::ChessGame();
}
}
