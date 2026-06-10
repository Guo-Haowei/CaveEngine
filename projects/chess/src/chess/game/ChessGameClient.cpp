#include "ChessGameClient.h"

#include "ChessGameSession.h"
#include "ChessIntent.h"
#include "ChessMatchAuthority.h"
#include "chess/core/MoveGen.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace chess {

using namespace ::cave;
using namespace ::cave::literals;
using namespace ::chess::core;

ChessGameClient::ChessGameClient(IHostServices& host,
                                 ChessGameSession& session,
                                 ChessMatchAuthority& auth)
    : session_(session)
    , auth_(auth)
    , board_view_(host)
    , piece_view_(host)
    , host_(host)
    , intent_dispatcher_(host.intentDispatcher())
    , debug_id_(MakeDebugId(this)) {

    intent_dispatcher_.addHandler<AuthMoveCommitted>(this);
    intent_dispatcher_.addHandler<AuthMoveRejected>(this);
    intent_dispatcher_.addHandler<AuthGameOver>(this);
}

ChessGameClient::~ChessGameClient() {
    intent_dispatcher_.removeHandler<AuthMoveCommitted>(this);
    intent_dispatcher_.removeHandler<AuthMoveRejected>(this);
    intent_dispatcher_.removeHandler<AuthGameOver>(this);
}

// @TODO: revisit this logic
void ChessGameClient::resetBoard() {
    replica_ = Position::Startpos();

    onPositionChange();
}

void ChessGameClient::onBoot() {
    board_view_.initialize();
    piece_view_.initialize();

    resetBoard();

    piece_view_.redrawPieces(replica_);
}

bool ChessGameClient::handleIntent(Intent& intent) {
    if (auto move_commited = dynamic_cast<AuthMoveCommitted*>(&intent)) {
        onMoveCommitted(move_commited->move());
        return true;
    }

    if (auto move_rejected = dynamic_cast<AuthMoveRejected*>(&intent)) {
        onMoveRejected(move_rejected->move());
        return true;
    }

    if (auto game_over = dynamic_cast<AuthGameOver*>(&intent)) {
        return true;
    }

    return false;
}

void ChessGameClient::onMoveCommitted(Move move) {
    piece_view_.applyMove(replica_, move);

    UndoState undo;
    replica_.MakeMove(move, undo);
    onPositionChange();

    session_.setPhase(SessionPhase::ResolvingMove);
}

void ChessGameClient::onMoveRejected(Move) {
    host_.log().Info(cave::LogChannel::Game, "Invalid move!");
}

void ChessGameClient::present() {
    board_view_.drawBoard();
}

void ChessGameClient::onPositionChange() {
    const MoveList moves = MoveGen::LegalMove(replica_);

    move_cache_.clear();
    for (Move move : moves) {
        move_cache_[move.from()].push_back(move);
    }
}

std::span<const Move> ChessGameClient::legalMoves(Square square) const {
    auto it = move_cache_.find(square);
    if (it == move_cache_.end()) {
        return {};
    }

    return it->second;
}

}  // namespace chess
