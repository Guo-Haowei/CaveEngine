#pragma once
#include "cave/game/IHostServices.h"

#include "chess/game/ChessTypes.h"

namespace chess {

class IPlayerAgent {
public:
    IPlayerAgent(core::Color side) noexcept
        : side_(side) {}

    virtual ~IPlayerAgent() = default;

    virtual void tick(cave::IHostServices& host) = 0;

    core::Color side() const { return side_; }

private:
    core::Color side_{};
};

}  // namespace chess
