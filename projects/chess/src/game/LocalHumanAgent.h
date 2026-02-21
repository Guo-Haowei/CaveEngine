#pragma once
#include "IntentInbox.h"
#include "IPlayerAgent.h"

namespace chess {

class ChessMatchAuthority;

class LocalHumanAgent final : public IPlayerAgent {
public:
    LocalHumanAgent(PlayerId p_player,
                    ChessMatchAuthority& p_authority)
        : m_player(p_player)
        , m_authority(p_authority) {
    }

    PlayerId GetPlayer() const override { return m_player; }

    IntentInbox& LocalInbox() { return m_local_inbox; }

    void Tick() override;

private:
    PlayerId m_player{};
    ChessMatchAuthority& m_authority;

    IntentInbox m_local_inbox;
};

}  // namespace chess