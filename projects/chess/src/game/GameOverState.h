#pragma once
#include "IChessGameState.h"

namespace chess {

class GameOverState final : public IChessGameState {
public:
    void Tick(cave::IHostServices& p_host, const cave::FrameTime& p_time) override;

#if USING(DEBUG_BUILD)
    const char* DebugName() const override { return "GamePlay"; }
#endif
};

}  // namespace chess
