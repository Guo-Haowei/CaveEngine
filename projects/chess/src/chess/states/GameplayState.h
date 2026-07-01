#pragma once
#include <memory>

#include "IChessGameState.h"

namespace chess {

class ChessGameSession;

class GameplayState final : public IChessGameState {
public:
    GameplayState() noexcept;
    ~GameplayState();

    void OnEnter(cave::IHostServices& p_host) override;
    void OnExit(cave::IHostServices& p_host) override;

    void Tick(cave::IHostServices& p_host, const cave::FrameTime& p_time) override;

#if USING(DEBUG_BUILD)
    const char* DebugName() const override { return "GamePlay"; }
#endif

private:
    std::unique_ptr<ChessGameSession> session_;
};

}  // namespace chess
