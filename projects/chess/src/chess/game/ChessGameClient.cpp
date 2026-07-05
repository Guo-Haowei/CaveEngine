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

ChessGameClient::ChessGameClient(IntentDispatcher& intent_bus,
                                 ChessGameSession& session,
                                 ChessMatchAuthority& auth)
    : m_intent_bus(intent_bus)
    , m_auth(auth)
    , m_session(session)
    , m_debug_id(MakeDebugId(this)) {

    m_intent_bus.addHandler<AuthMoveCommitted>(this);
    m_intent_bus.addHandler<AuthMoveRejected>(this);
    m_intent_bus.addHandler<AuthGameOver>(this);
}

ChessGameClient::~ChessGameClient() {
    m_intent_bus.removeHandler<AuthMoveCommitted>(this);
    m_intent_bus.removeHandler<AuthMoveRejected>(this);
    m_intent_bus.removeHandler<AuthGameOver>(this);
}

// @TODO: revisit this logic
void ChessGameClient::resetBoard() {
    m_replica = Position::Startpos();

    onPositionChange();
}

void ChessGameClient::onBoot(SceneQuery& query) {
    m_board_view.initialize(query);
    m_piece_view.initialize(query);

    resetBoard();

    m_piece_view.redrawPieces(query, m_replica);
}

bool ChessGameClient::handleIntent(Intent& intent) {
    if (auto move_commited = dynamic_cast<AuthMoveCommitted*>(&intent)) {
        // onMoveCommitted(move_commited->move());
        return true;
    }

    if (auto move_rejected = dynamic_cast<AuthMoveRejected*>(&intent)) {
        // onMoveRejected(move_rejected->move());
        return true;
    }

    if (auto game_over = dynamic_cast<AuthGameOver*>(&intent)) {
        return true;
    }

    return false;
}

void ChessGameClient::onMoveCommitted(SceneQuery& query, Move move) {
    m_piece_view.applyMove(query, m_replica, move);

    UndoState undo;
    m_replica.MakeMove(move, undo);
    onPositionChange();

    m_session.setPhase(SessionPhase::ResolvingMove);
}

void ChessGameClient::onMoveRejected(SceneQuery&, Move) {
    LOG_INFO(LogChannel::Game, "Invalid move!");
}

void ChessGameClient::present(SceneQuery& query) {
    m_board_view.drawBoard(query);
}

void ChessGameClient::onPositionChange() {
    const MoveList moves = MoveGen::LegalMove(m_replica);

    m_move_cache.clear();
    for (Move move : moves) {
        m_move_cache[move.from()].push_back(move);
    }
}

std::span<const Move> ChessGameClient::legalMoves(Square square) const {
    auto it = m_move_cache.find(square);
    if (it == m_move_cache.end()) {
        return {};
    }

    return it->second;
}

}  // namespace chess
