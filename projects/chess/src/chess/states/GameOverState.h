#pragma once
#include "IChessGameState.h"

namespace chess {

#if 0
class GameOverState final : public IChessGameState {
public:
    void Tick(cave::IHostServices& p_host, const cave::FrameTime& p_time) override;

#if USING(DEBUG_BUILD)
    const char* debugName() const override { return "GamePlay"; }
#endif
};
#endif

}  // namespace chess
