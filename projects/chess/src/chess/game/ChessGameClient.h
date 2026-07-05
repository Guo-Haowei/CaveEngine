#pragma once
#include <cstdint>
#include <span>
#include <unordered_map>
#include <unordered_set>

#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/intent/IIntentHandler.h"
#include "cave/runtime/intent/IntentDispatcher.h"

#include "chess/presentation/ChessBoardView.h"
#include "chess/presentation/ChessPieceView.h"

namespace chess {

class ChessGameSession;
class ChessMatchAuthority;

class ChessGameClient : public cave::IIntentHandler {
public:
    ChessGameClient(cave::IntentDispatcher& intent_bus,
                    ChessGameSession& session,
                    ChessMatchAuthority& auth);
    ~ChessGameClient();

    void onBoot(cave::SceneQuery& query);

    void present(cave::SceneQuery& query);

    std::span<const core::Move> legalMoves(core::Square square) const;

    const core::Position& replica() const { return m_replica; }

    bool handleIntent(cave::Intent& intent) override;

    cave::DebugId debugId() const override { return m_debug_id; }

    ChessBoardView& board_view() { return m_board_view; }

private:
    void onMoveCommitted(cave::SceneQuery& query, core::Move move);
    void onMoveRejected(cave::SceneQuery& query, core::Move move);

    void onPositionChange();

    void resetBoard();

    cave::IntentDispatcher& m_intent_bus;
    ChessMatchAuthority& m_auth;
    ChessGameSession& m_session;
    const cave::DebugId m_debug_id;

    ChessBoardView m_board_view;
    ChessPieceView m_piece_view;

    core::Position m_replica;
    std::unordered_map<core::Square, std::vector<core::Move>> m_move_cache;
};

}  // namespace chess