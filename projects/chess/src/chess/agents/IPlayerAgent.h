#pragma once
#include "chess/game/ChessTypes.h"

#include "cave/game/IHostServices.h"

namespace chess {

class IPlayerAgent {
public:
    IPlayerAgent(PlayerId p_player) noexcept
        : player_(p_player) {}

    virtual ~IPlayerAgent() = default;

    virtual void tick(cave::IHostServices& host) = 0;

    PlayerId playerId() const { return player_; }

private:
    PlayerId player_{};
};

}  // namespace chess
