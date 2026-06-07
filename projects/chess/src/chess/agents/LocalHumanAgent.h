#pragma once
#include "chess/agents/IPlayerAgent.h"

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

    void Tick(cave::IHostServices& p_host) override;

private:
    PlayerId m_player{};
    ChessMatchAuthority& m_auth;
};

}  // namespace chess