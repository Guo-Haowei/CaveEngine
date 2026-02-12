#pragma once
#include "cave/game/IGameModule.h"

namespace cave {

class ChessGame final : public IGameModule {
public:
    void RegisterTypes(IHostServices& p_host) override;

    void RegisterSystems(IHostServices& p_host) override;

    void CreateWorld(World& world, IHostServices& p_host, const GameInitDesc& init) override;

    void Tick(World& world, IHostServices& p_host, const FrameTime& time) override;
};

}  // namespace cave

extern "C" {
__declspec(dllexport) cave::IGameModule* CreateGameModule() {
    return new ::cave::ChessGame();
}
}
