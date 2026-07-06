#pragma once
#include <memory>

#include "IChessGameState.h"

namespace chess {

class ChessGameSession;

class GameplayState final : public IChessGameState {
public:
    GameplayState(cave::IntentBus& intent_bus) noexcept;
    ~GameplayState() override;

    void onEnter(cave::SceneContext& ctx) override;
    void onExit() override;

    void tick(cave::SceneContext& ctx, float dt) override;

#if USING(DEBUG_BUILD)
    const char* debugName() const override { return "GamePlay"; }
#endif

private:
    std::unique_ptr<ChessGameSession> session_;
};

}  // namespace chess
