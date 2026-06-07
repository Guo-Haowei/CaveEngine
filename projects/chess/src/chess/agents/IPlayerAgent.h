#pragma once
#include "chess/game/ChessTypes.h"

#include "cave/game/IHostServices.h"

namespace chess {

class IPlayerAgent {
public:
    virtual ~IPlayerAgent() = default;

    virtual void Tick(cave::IHostServices& p_host) = 0;

    virtual PlayerId GetPlayer() const = 0;
};

}  // namespace chess
