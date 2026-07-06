#pragma once
#include "chess/agents/IPlayerAgent.h"

namespace chess {

class LocalHumanAgent final : public IPlayerAgent {
public:
    using IPlayerAgent::IPlayerAgent;

    void tick(cave::IntentBus& intent_bus) override;
};

}  // namespace chess