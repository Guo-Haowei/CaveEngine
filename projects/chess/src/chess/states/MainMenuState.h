#pragma once
#include "IChessGameState.h"

namespace chess {

class MainMenuState final : public IChessGameState {
public:
    void OnEnter(cave::IHostServices& p_host) override;

    void Tick(cave::IHostServices& p_host, const cave::FrameTime& p_time) override;

#if USING(DEBUG_BUILD)
    const char* debugName() const override { return "MainMenu"; }
#endif
};

}  // namespace chess
