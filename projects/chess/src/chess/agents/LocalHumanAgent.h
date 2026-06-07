#pragma once
#include "chess/agents/IPlayerAgent.h"

namespace chess {

class LocalHumanAgent final : public IPlayerAgent {
public:
    using IPlayerAgent::IPlayerAgent;

    void tick(cave::IHostServices& host) override;
};

}  // namespace chess