#pragma once
#include "cave/core/memory/Pointer.h"
#include "cave/runtime/game/IGameMode.h"
#include "cave/runtime/intent/IIntentHandler.h"
#include "cave/runtime/intent/IntentBus.h"
#include "cave/runtime/scene/SceneRuntime.h"

namespace chess {

class ChessGameSession;

class ChessGameMode final : public cave::IGameMode {
public:
    ChessGameMode(cave::SceneRuntime& runtime,
                  cave::IntentBus& intent_bus);
    ~ChessGameMode() override;

    void onEnter() final;
    void onExit() final;
    void tick(float dt) final;

private:
    cave::Owner<ChessGameSession> m_session;
};

}  // namespace chess
