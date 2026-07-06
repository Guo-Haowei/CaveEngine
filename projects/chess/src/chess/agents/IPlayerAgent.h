#pragma once
#include "cave/runtime/intent/IntentBus.h"
#include "chess/game/ChessTypes.h"

namespace chess {

class IPlayerAgent {
public:
    IPlayerAgent(core::Color side) noexcept
        : m_side(side) {}

    virtual ~IPlayerAgent() = default;

    virtual void tick(cave::IntentBus& intent_bus) = 0;

    core::Color side() const { return m_side; }

private:
    core::Color m_side{};
};

}  // namespace chess
