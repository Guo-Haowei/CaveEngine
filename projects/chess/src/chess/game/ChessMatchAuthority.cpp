#include "ChessMatchAuthority.h"

#include "ChessIntent.h"
#include "chess/core/MoveGen.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"

namespace chess {

using core::Color;
using core::MoveGen;
using core::MoveList;
using core::Position;

ChessMatchAuthority::ChessMatchAuthority(cave::IHostServices& p_host)
    : m_intent(p_host.Intent())
    , m_debug_id(cave::MakeDebugId(this)) {
    m_intent.AddHandler<ChessMoveIntent>(this);
    m_pos = Position::Startpos();
}

ChessMatchAuthority::~ChessMatchAuthority() {
    m_intent.RemoveHandler<ChessMoveIntent>(this);
}

bool ChessMatchAuthority::HandleIntent(cave::Intent& p_intent) {
    if (auto intent = dynamic_cast<ChessMoveIntent*>(&p_intent)) {
        TryCommitMove(intent->player(), intent->move());
        return true;
    }

    return false;
}

bool ChessMatchAuthority::TryCommitMove(PlayerId p_player_id,
                                        core::Move p_move) {

    if (m_pos.SideToMove() != p_player_id) {
        return false;
    }

    core::UndoState undo;
    Position copy = m_pos;
    const bool ok = copy.MakeMove(p_move, undo);
    if (!ok) {
        m_intent.Queue<AuthMoveRejected>(p_player_id, p_move);
        return false;
    }

    m_pos = copy;
    m_intent.Queue<AuthMoveCommitted>(p_player_id, p_move);

    // @TODO: figure out if draw or not
    const MoveList moves = MoveGen::LegalMove(m_pos);
    if (moves.Empty()) {
        m_game_over = true;
        m_intent.Queue<AuthGameOver>(p_player_id, p_move);
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
