#pragma once
#include "IChessGameState.h"

namespace chess {

class MainMenuState final : public IChessGameState {
public:
    using IChessGameState::IChessGameState;

    void onEnter(cave::SceneContext& ctx) override;

    void tick(cave::SceneContext& ctx, float dt) override;

#if USING(DEBUG_BUILD)
    const char* debugName() const override { return "MainMenu"; }
#endif
};

}  // namespace chess
