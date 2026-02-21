#pragma once
#include <memory>
#include "cave/core/typedefs.h"
#include "cave/game/IGameModule.h"
#include "cave/runtime/controller/GridSelectController.h"

namespace chess {

class ChessGameSession;

class ChessGameModule final : public cave::IGameModule {
public:
    ChessGameModule();
    ~ChessGameModule();

    void OnModuleLoaded(cave::IHostServices& p_host) override;
    void OnModuleUnloaded(cave::IHostServices& p_host) override;

    void OnGameBegin(cave::IHostServices& p_host) override;
    void OnGameEnd(cave::IHostServices& p_host) override;

    void Tick(cave::IHostServices& p_host, const cave::FrameTime& p_time) override;

private:
    void SpawnObjects(cave::IHostServices& p_host);

    std::unique_ptr<ChessGameSession> m_session;
};

}  // namespace chess

extern "C" {
CAVE_API cave::IGameModule* CreateGameModule() {
    return new ::chess::ChessGameModule();
}
}
