#pragma once
#include "chess/agents/IPlayerAgent.h"

namespace chess {

class ChessGameClient;

class ChessAIAgent final : public IPlayerAgent {
public:
    explicit ChessAIAgent(core::Color player,
                          ChessGameClient& client) noexcept
        : IPlayerAgent(player)
        , client_(client) {
    }

    void tick(cave::IntentBus& intent_bus) override;

private:
    ChessGameClient& client_;
};

}  // namespace chess
