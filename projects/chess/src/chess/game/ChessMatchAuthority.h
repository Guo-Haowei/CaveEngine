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
    ChessMatchAuthority(cave::IHostServices& host);
    ~ChessMatchAuthority();

    bool gameOver() const { return game_over_; }

    bool handleIntent(cave::Intent& intent) override;

    cave::DebugId debugId() const override { return debug_id_; }

    Color sideToMove() const { return pos_.SideToMove(); }

private:
    bool tryCommitMove(Color side, core::Move move);

    void offerDraw(Color side);
    void resign(Color side);

    core::Position pos_;

    bool game_over_{ false };

    cave::IntentDispatcher& intent_dispatcher;
    const cave::DebugId debug_id_;
};

}  // namespace chess
