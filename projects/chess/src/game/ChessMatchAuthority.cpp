#include "ChessMatchAuthority.h"

#include "core/MoveGen.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"

namespace chess {

using core::Color;
using core::Position;
using core::MoveGen;
using core::MoveList;

ChessMatchAuthority::ChessMatchAuthority(cave::IHostServices& p_host)
    : m_intent(p_host.Intent())
    , m_debug_id(cave::MakeDebugId(this)) {
    m_intent.AddHandler<MoveIntent>(this);
    m_pos = Position::Startpos();
}

ChessMatchAuthority::~ChessMatchAuthority() {
    m_intent.RemoveHandler<MoveIntent>(this);
}

bool ChessMatchAuthority::HandleIntent(cave::Intent& p_intent) {
    if (auto intent = dynamic_cast<MoveIntent*>(&p_intent)) {
        TryCommitMove(intent->player, intent->mv);
        return true;
    }

    return false;
}

bool ChessMatchAuthority::Pop(AuthorityEvent& p_out) {
    if (m_events.empty()) return false;

    p_out = m_events.front();
    m_events.pop_front();
    return true;
}

bool ChessMatchAuthority::TryCommitMove(PlayerId p_player_id,
                                        core::Move p_move) {

    const PlayerId side = std::to_underlying(m_pos.SideToMove());
    if (side != p_player_id) {
        return false;
    }

    core::UndoState undo;
    Position copy = m_pos;
    const bool ok = copy.MakeMove(p_move, undo);
    if (!ok) {
        m_events.push_back({ AuthorityEventType::MoveRejected, p_player_id, p_move });
        return false;
    }

    m_pos = copy;
    m_events.push_back({ AuthorityEventType::MoveCommitted, p_player_id, p_move });

    // @TODO: figure out if draw or not
    const MoveList moves = MoveGen::LegalMove(m_pos);
    if (moves.Empty()) {
        m_game_over = true;
        m_events.push_back({ AuthorityEventType::GameOver, p_player_id, p_move });
    }

    return true;
}

void ChessMatchAuthority::OfferDraw(PlayerId p_player_id) {
    (void)p_player_id;
}

void ChessMatchAuthority::Resign(PlayerId p_player_id) {
    (void)p_player_id;
}

}  // namespace chess
