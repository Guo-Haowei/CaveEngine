#pragma once
#include "cave/core/memory/Pointer.h"
#include "cave/runtime/game/IGameMode.h"
#include "cave/runtime/intent/IIntentHandler.h"
#include "cave/runtime/intent/IntentBus.h"
#include "cave/runtime/scene/SceneRuntime.h"

namespace chess {

class IChessGameState;

class ChessGameMode final : public cave::IGameMode,
                            public cave::IIntentHandler {
public:
    ChessGameMode(cave::SceneRuntime& runtime,
                  cave::IntentBus& intent_bus);
    ~ChessGameMode();

    void onEnter() final;
    void onExit() final;
    void tick(float dt) final;

    bool handleIntent(cave::Intent& intent) override;

    cave::DebugId debugId() const override { return m_debug_id; }

private:
    void commitStateChange(cave::Owner<IChessGameState>&& new_state);

    cave::IntentBus& m_intent_bus;
    cave::SceneRuntime& m_runtime;
    const cave::DebugId m_debug_id;

    std::unique_ptr<IChessGameState> m_state;
};

}  // namespace chess
