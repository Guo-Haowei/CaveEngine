#pragma once
#include <deque>

#include "core/Position.h"
#include "IPlayerAgent.h"

#include "cave/runtime/intent/IIntentHandler.h"
#include "cave/runtime/intent/IntentDispatcher.h"

namespace chess {

class ChessMatchAuthority : public cave::IIntentHandler {
    using Color = core::Color;

public:
    ChessMatchAuthority(cave::IHostServices& p_host);
    ~ChessMatchAuthority();

    bool GameOver() const { return m_game_over; }

    bool HandleIntent(cave::Intent& p_intent) override;

    cave::DebugId GetDebugId() const override { return m_debug_id; }

    PlayerId CurrentPlayer() const { return (PlayerId)m_pos.SideToMove(); }

private:
    bool TryCommitMove(PlayerId p_player_id,
                       core::Move p_move);

    void OfferDraw(PlayerId p_player_id);
    void Resign(PlayerId p_player_id);

    core::Position m_pos;

    bool m_game_over = false;

    cave::IntentDispatcher& m_intent;
    const cave::DebugId m_debug_id;
};

}  // namespace chess
