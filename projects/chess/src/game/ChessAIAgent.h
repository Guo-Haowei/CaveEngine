#pragma once
#include "IPlayerAgent.h"

namespace chess {

class ChessGameClient;
class ChessMatchAuthority;

class ChessAIAgent : public IPlayerAgent {
public:
    explicit ChessAIAgent(PlayerId p_player,
                          ChessMatchAuthority& p_authority,
                          ChessGameClient& p_client) noexcept
        : m_player(p_player)
        , m_auth(p_authority)
        , m_client(p_client) {
    }

    PlayerId GetPlayer() const override { return m_player; }

    void Tick() override;

private:
    PlayerId m_player{};
    ChessMatchAuthority& m_auth;
    ChessGameClient& m_client;
};

}  // namespace chess
