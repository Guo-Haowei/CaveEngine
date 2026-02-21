#pragma once
#include "core/Position.h"
#include "IntentInbox.h"
#include "IPlayerAgent.h"

namespace chess {

class IAuthoritySink {
public:
    virtual ~IAuthoritySink() = default;
    virtual void OnMoveCommitted(const core::Move& p_mv) = 0;
    // Add reject move
    // Add draw
    // Add resign
};

class ChessMatchAuthority {
    using Color = core::Color;

public:
    void SetSink(IAuthoritySink* sink) { m_sink = sink; }

    IntentInbox& Inbox(PlayerId p_player_id) {
        return m_inbox[p_player_id];
    }

    void Tick();

private:
    bool TryCommitMove(PlayerId p_player_id,
                       core::Move p_move);

    void OfferDraw(PlayerId p_player_id);
    void Resign(PlayerId p_player_id);

    bool HandleIntent(PlayerId p, const PlayerIntent& p_intent);

private:
    core::Position m_pos;

    IntentInbox m_inbox[2]{};
    IAuthoritySink* m_sink = nullptr;
};

}  // namespace chess
