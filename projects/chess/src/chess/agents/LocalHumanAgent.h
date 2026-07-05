#pragma once
#include "chess/agents/IPlayerAgent.h"

namespace chess {

class LocalHumanAgent final : public IPlayerAgent {
public:
    using IPlayerAgent::IPlayerAgent;

    void tick(cave::SceneContext& ctx) override;
};

}  // namespace chess