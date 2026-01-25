#include "ChessMode.h"

namespace cave {

void ChessGameMode::OnEnter(GameSession& p_session) {
    (void)p_session;
    printf("ChessGameMode::OnEnter\n");
}

void ChessGameMode::OnExit(GameSession& p_session) {
    (void)p_session;
    printf("ChessGameMode::OnExit\n");
}

void ChessGameMode::Tick(GameSession& p_session, const GameFrameTime& p_time) {
    (void)p_session;
    (void)p_time;
    printf("ChessGameMode::Tick\n");
}

}  // namespace cave
