#pragma once
#include <deque>

#include "cave/runtime/intent/IIntentHandler.h"
#include "cave/runtime/intent/IntentDispatcher.h"

#include "chess/agents/IPlayerAgent.h"
#include "chess/core/Position.h"

namespace chess {

class ChessMatchAuthority : public cave::IIntentHandler {
    using Color = core::Color;

public:
    ChessMatchAuthority(cave::IntentDispatcher& intent_bus);
    ~ChessMatchAuthority();

    bool gameOver() const { return m_game_over; }

    bool handleIntent(cave::Intent& intent) override;

    cave::DebugId debugId() const override { return m_debug_id; }

    Color sideToMove() const { return m_pos.sideToMove(); }

private:
    bool tryCommitMove(Color side, core::Move move);

    void offerDraw(Color side);
    void resign(Color side);

    cave::IntentDispatcher& m_intent_bus;
    const cave::DebugId m_debug_id;

    core::Position m_pos;
    bool m_game_over{ false };
};

}  // namespace chess
