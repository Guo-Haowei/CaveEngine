#pragma once
#include "cave/game/IHostServices.h"

#include "chess/game/ChessTypes.h"

namespace chess {

class IPlayerAgent {
public:
    IPlayerAgent(PlayerId player) noexcept
        : player_(player) {}

    virtual ~IPlayerAgent() = default;

    virtual void tick(cave::IHostServices& host) = 0;

    PlayerId playerId() const { return player_; }

private:
    PlayerId player_{};
};

}  // namespace chess
