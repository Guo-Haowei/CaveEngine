#pragma once
#include "IntentInbox.h"
#include "IPlayerAgent.h"

namespace chess {

class ChessMatchAuthority;

class LocalHumanAgent final : public IPlayerAgent {
public:
    explicit LocalHumanAgent(PlayerId p_player,
                             ChessMatchAuthority& p_authority) noexcept
        : m_player(p_player)
        , m_auth(p_authority) {
    }

    PlayerId GetPlayer() const override { return m_player; }

    IntentInbox& LocalInbox() { return m_local_inbox; }

    void Tick() override;

private:
    PlayerId m_player{};
    ChessMatchAuthority& m_auth;

    IntentInbox m_local_inbox;
};

}  // namespace chess