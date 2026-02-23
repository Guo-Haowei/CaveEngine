#include "ChessMatchAuthority.h"

namespace chess {

using core::Color;
using core::Position;

ChessMatchAuthority::ChessMatchAuthority() {
    m_pos = Position::Startpos();
}

void ChessMatchAuthority::Tick() {
    const int player = (m_pos.SideToMove() == Color::White) ? 0 : 1;

    PlayerIntent intent;
    while (m_inbox[player].Pop(intent)) {
        if (HandleIntent(player, intent)) {
            break;
        }
    }
}

bool ChessMatchAuthority::Pop(AuthorityEvent& p_out) {
    if (m_events.empty()) return false;

    p_out = m_events.front();
    m_events.pop_front();
    return true;
}

bool ChessMatchAuthority::TryCommitMove(PlayerId p_player_id,
                                        core::Move p_move) {
    core::UndoState undo;
    const bool ok = m_pos.MakeMove(p_move, undo);

    m_events.push_back({ AuthorityEventType ::MoveCommitted, p_player_id, p_move });
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
                return false;  // state unchanged
            }
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
