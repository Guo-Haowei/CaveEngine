#include "ChessGameMode.h"

#include <cave/core/Print.h>

#include <cave/core/math/Matrix.h>

namespace cave {

void ChessGameMode::OnEnter(GameSession& p_session) {
    unused(p_session);
    LOG_VERBOSE("ChessGameMode::OnEnter");
}

void ChessGameMode::OnExit(GameSession& p_session) {
    unused(p_session);
    LOG_VERBOSE("ChessGameMode::OnExit");
}

void ChessGameMode::Tick(GameSession& p_session, const GameFrameTime& p_time) {
    unused(p_session);
    unused(p_time);
}

}  // namespace cave
