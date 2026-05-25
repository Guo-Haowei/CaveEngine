#pragma once
#include <cstdint>
#include <memory>

#include "cave/game/IGameMode.h"

namespace chess {

using cave::FrameTime;
using cave::IHostServices;

class ChessGameSession;
class IChessGameState;

class ChessGameMode final : public cave::IGameMode {
public:
    ChessGameMode();
    ~ChessGameMode();

    void OnEnter(IHostServices& p_host) final;
    void OnExit(IHostServices& p_host) final;
    void Tick(IHostServices& p_host, const FrameTime& p_time) final;

    void SetPendingState(std::unique_ptr<IChessGameState>&& p_pending);

private:
    void CommitStateChange(cave::IHostServices& p_host);

    std::unique_ptr<IChessGameState> m_current_state;
    std::unique_ptr<IChessGameState> m_pending_state;

    std::unique_ptr<ChessGameSession> m_session;
};

}  // namespace chess
