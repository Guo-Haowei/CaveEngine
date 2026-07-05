#pragma once
#include <cstdint>
#include <memory>

#include "cave/game/IGameMode.h"
#include "cave/runtime/intent/IIntentHandler.h"
#include "cave/runtime/intent/IntentBus.h"

namespace chess {

class IChessGameState;

class ChessGameMode final : public cave::IGameMode,
                            public cave::IIntentHandler {
public:
    ChessGameMode(cave::SceneContext& ctx);
    ~ChessGameMode();

    void onEnter(cave::SceneContext& ctx) final;
    void onExit() final;
    void tick(cave::SceneContext& ctx, float dt) final;

    bool handleIntent(cave::Intent& intent) override;

    cave::DebugId debugId() const override { return m_debug_id; }

private:
    void commitStateChange(cave::SceneContext& ctx, std::unique_ptr<IChessGameState>&& new_state);

    cave::IntentBus& m_intent_bus;
    const cave::DebugId m_debug_id;

    std::unique_ptr<IChessGameState> m_state;
};

}  // namespace chess
