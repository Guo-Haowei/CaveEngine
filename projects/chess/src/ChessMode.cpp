#include "ChessMode.h"

namespace cave {

void ChessMode::OnEnter(GameSession& p_session) {
    (void)p_session;
    printf("ChessMode::OnEnter\n");
}

void ChessMode::OnExit(GameSession& p_session) {
    (void)p_session;
    printf("ChessMode::OnExit\n");
}

void ChessMode::Tick(GameSession& p_session, const GameFrameTime& p_time) {
    (void)p_session;
    (void)p_time;
    printf("ChessMode::Tick\n");
}

}  // namespace cave
