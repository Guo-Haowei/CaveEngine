#include "ChessMatchAuthority.h"

#include "ChessIntent.h"
#include "chess/core/MoveGen.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"

namespace chess {

using namespace ::cave;
using namespace ::chess::core;

ChessMatchAuthority::ChessMatchAuthority(IHostServices& host)
    : intent_dispatcher(host.intentDispatcher())
    , debug_id_(MakeDebugId(this)) {
    intent_dispatcher.addHandler<ChessMoveIntent>(this);
    pos_ = Position::Startpos();
}

ChessMatchAuthority::~ChessMatchAuthority() {
    intent_dispatcher.removeHandler<ChessMoveIntent>(this);
}

bool ChessMatchAuthority::handleIntent(cave::Intent& intent) {
    if (auto move_intent = dynamic_cast<ChessMoveIntent*>(&intent)) {
        tryCommitMove(move_intent->side(), move_intent->move());
        return true;
    }

    return false;
}

bool ChessMatchAuthority::tryCommitMove(Color side, Move move) {
    if (pos_.SideToMove() != side) {
        return false;
    }

    core::UndoState undo;
    Position copy = pos_;
    const bool ok = copy.MakeMove(move, undo);
    if (!ok) {
        intent_dispatcher.queue<AuthMoveRejected>(side, move);
        return false;
    }

    pos_ = copy;
    intent_dispatcher.queue<AuthMoveCommitted>(side, move);

    // @TODO: figure out if draw or not
    const MoveList moves = MoveGen::LegalMove(pos_);
    if (moves.empty()) {
        game_over_ = true;
        intent_dispatcher.queue<AuthGameOver>(side, move);
    }

    return true;
}

void ChessMatchAuthority::offerDraw(Color) {
}

void ChessMatchAuthority::resign(Color) {
}

}  // namespace chess
