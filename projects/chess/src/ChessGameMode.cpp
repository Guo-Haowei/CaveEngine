#include "ChessGameMode.h"

#include <cave/runtime/core/Print.h>

namespace cave {

void ChessGameMode::OnEnter(GameSession& p_session) {
    unused(p_session);
    LOG_VERBOSE("ChessGameMode::OnEnter\n");
}

void ChessGameMode::OnExit(GameSession& p_session) {
    unused(p_session);
    LOG_VERBOSE ("ChessGameMode::OnExit\n");
}

void ChessGameMode::Tick(GameSession& p_session, const GameFrameTime& p_time) {
    unused(p_session);
    unused(p_time);
    LOG_VERBOSE("ChessGameMode::Tick\n");
}

}  // namespace cave
