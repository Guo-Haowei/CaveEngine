#include "ChessMatchAuthority.h"

namespace chess {

using core::Color;

void ChessMatchAuthority::Tick() {
    const int player = (m_pos.SideToMove() == Color::White) ? 0 : 1;

    PlayerIntent intent;
    while (m_inbox[player].Pop(intent)) {
        if (HandleIntent(player, intent)) break;
    }
}

bool ChessMatchAuthority::TryCommitMove(PlayerId p_player_id,
                                        core::Move p_move) {
    (void)p_player_id;

    core::UndoState undo;
    const bool ok = m_pos.MakeMove(p_move, undo);
    return ok;
}

void ChessMatchAuthority::OfferDraw(PlayerId p_player_id) {
    (void)p_player_id;
}

void ChessMatchAuthority::Resign(PlayerId p_player_id) {
    (void)p_player_id;
}

bool ChessMatchAuthority::HandleIntent(PlayerId p_player_id, const PlayerIntent& p_intent) {
    switch (p_intent.type) {
        case IntentType::AttemptMove:
            if (!TryCommitMove(p_player_id, p_intent.move)) {
                __debugbreak();
                if (m_sink) {
                    // @NOTE: only player controller for now,
                    // invalid moves should be rejected already
                    // m_sink->OnMoveRejected(p, i.move);
                }
                return false;  // state unchanged
            }
            if (m_sink) {
                m_sink->OnMoveCommitted(p_intent.move);
            }
            // advance side-to-move (real code should do this based on position)
            return true;

        case IntentType::OfferDraw:
            OfferDraw(p_player_id);
            return true;

        case IntentType::Resign:
            Resign(p_player_id);
            return true;
    }
    return false;
}

}  // namespace chess
