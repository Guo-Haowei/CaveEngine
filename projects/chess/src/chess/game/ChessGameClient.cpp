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
    : presenter_(host)
    , session_(session)
    , auth_(auth)
    , host_(host)
    , intent_dispatcher_(host.intentDispatcher())
    , debug_id_(MakeDebugId(this)) {

    intent_dispatcher_.AddHandler<AuthMoveCommitted>(this);
    intent_dispatcher_.AddHandler<AuthMoveRejected>(this);
    intent_dispatcher_.AddHandler<AuthGameOver>(this);
}

ChessGameClient::~ChessGameClient() {
    intent_dispatcher_.RemoveHandler<AuthMoveCommitted>(this);
    intent_dispatcher_.RemoveHandler<AuthMoveRejected>(this);
    intent_dispatcher_.RemoveHandler<AuthGameOver>(this);
}

// @TODO: revisit this logic
void ChessGameClient::resetBoard() {
    replica_ = Position::Startpos();

    onPositionChange();
}

void ChessGameClient::onBoot() {
    presenter_.onBoot();
    resetBoard();

    presenter_.redrawBoard(replica_);
}

bool ChessGameClient::HandleIntent(Intent& p_intent) {
    if (auto intent = dynamic_cast<AuthMoveCommitted*>(&p_intent)) {
        onMoveCommitted(intent->move());
        return true;
    }

    if (auto intent = dynamic_cast<AuthMoveRejected*>(&p_intent)) {
        onMoveRejected(intent->move());
        return true;
    }

    if (auto intenti = dynamic_cast<AuthGameOver*>(&p_intent)) {
        return true;
    }

    return false;
}

void ChessGameClient::onMoveCommitted(Move p_mv) {
    presenter_.applyMove(p_mv);

    UndoState undo;
    replica_.MakeMove(p_mv, undo);
    onPositionChange();

    session_.SetState(SessionState::ResolvingMove);
}

void ChessGameClient::onMoveRejected(Move) {
    host_.log().Info(cave::LogChannel::Game, "Invalid move!");
}

void ChessGameClient::present() {
    presenter_.present();
}

void ChessGameClient::onPositionChange() {
    const MoveList moves = MoveGen::LegalMove(replica_);

    move_cache_.clear();
    for (Move move : moves) {
        move_cache_[move.From()].push_back(move);
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
