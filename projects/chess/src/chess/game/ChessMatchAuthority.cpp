#include "ChessMatchAuthority.h"

#include "ChessIntent.h"
#include "chess/core/MoveGen.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"

namespace chess {

using namespace ::cave;
using namespace ::chess::core;

ChessMatchAuthority::ChessMatchAuthority(IntentBus& intent_bus)
    : m_intent_bus(intent_bus)
    , m_debug_id(MakeDebugId(this)) {
    m_intent_bus.addHandler<ChessMoveIntent>(this);
    m_pos = Position::Startpos();
}

ChessMatchAuthority::~ChessMatchAuthority() {
    m_intent_bus.removeHandler<ChessMoveIntent>(this);
}

bool ChessMatchAuthority::handleIntent(Intent& intent) {
    if (auto move_intent = dynamic_cast<ChessMoveIntent*>(&intent)) {
        tryCommitMove(move_intent->side(), move_intent->move());
        return true;
    }

    return false;
}

bool ChessMatchAuthority::tryCommitMove(Color side, Move move) {
    if (m_pos.sideToMove() != side) {
        return false;
    }

    core::UndoState undo;
    Position copy = m_pos;
    const bool ok = copy.MakeMove(move, undo);
    if (!ok) {
        m_intent_bus.queue<AuthMoveRejected>(side, move);
        return false;
    }

    m_pos = copy;
    m_intent_bus.queue<AuthMoveCommitted>(side, move);

    // @TODO: figure out if draw or not
    const MoveList moves = MoveGen::LegalMove(m_pos);
    if (moves.empty()) {
        m_game_over = true;
        m_intent_bus.queue<AuthGameOver>(side, move);
    }

    return true;
}

void ChessMatchAuthority::offerDraw(Color) {
}

void ChessMatchAuthority::resign(Color) {
}

}  // namespace chess
