#pragma once
#include <cstdint>
#include <span>
#include <unordered_map>
#include <unordered_set>

#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/intent/IIntentHandler.h"
#include "cave/runtime/intent/IntentBus.h"

#include "chess/presentation/ChessBoardView.h"
#include "chess/presentation/ChessPieceView.h"

namespace chess {

class ChessGameSession;
class ChessMatchAuthority;

class ChessGameClient : public cave::IIntentHandler {
public:
    ChessGameClient(cave::IntentBus& intent_bus,
                    cave::Scene& scene,
                    ChessGameSession& session,
                    ChessMatchAuthority& auth);
    ~ChessGameClient();

    void onBoot();

    void present();

    std::span<const core::Move> legalMoves(core::Square square) const;

    const core::Position& replica() const { return m_replica; }

    bool handleIntent(cave::Intent& intent) override;

    cave::DebugId debugId() const override { return m_debug_id; }

    ChessBoardView& board_view() { return m_board_view; }

private:
    void onMoveCommitted(core::Move move);
    void onMoveRejected(core::Move move);

    void onPositionChange();

    void resetBoard();

    cave::IntentBus& m_intent_bus;
    cave::SceneQuery m_query;

    ChessMatchAuthority& m_auth;
    ChessGameSession& m_session;

    ChessBoardView m_board_view;
    ChessPieceView m_piece_view;

    const cave::DebugId m_debug_id;

    core::Position m_replica;
    std::unordered_map<core::Square, std::vector<core::Move>> m_move_cache;
};

}  // namespace chess