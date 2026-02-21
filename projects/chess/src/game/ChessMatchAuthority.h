#pragma once
#include "core/Position.h"
#include "IntentInbox.h"
#include "IPlayerAgent.h"

namespace chess {

enum class AuthorityEventType : uint8_t {
    MoveCommitted,
    MoveRejected,
    GameOver,
};

struct AuthorityEvent {
    AuthorityEventType type{};
    PlayerId player{};
    core::Move move{};
};

class ChessMatchAuthority {
    using Color = core::Color;

public:
    ChessMatchAuthority();

    IntentInbox& Inbox(PlayerId p_player_id) {
        return m_inbox[p_player_id];
    }

    void Tick();

    bool Pop(AuthorityEvent& p_out);

private:
    bool TryCommitMove(PlayerId p_player_id,
                       core::Move p_move);

    void OfferDraw(PlayerId p_player_id);
    void Resign(PlayerId p_player_id);

    bool HandleIntent(PlayerId p, const PlayerIntent& p_intent);

    core::Position m_pos;

    IntentInbox m_inbox[2]{};

    std::deque<AuthorityEvent> m_events;
};

}  // namespace chess
