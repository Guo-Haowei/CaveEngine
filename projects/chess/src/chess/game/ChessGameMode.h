#pragma once
#include <cstdint>
#include <memory>

#include "cave/game/IGameMode.h"
#include "cave/runtime/intent/IIntentHandler.h"
#include "cave/runtime/intent/IntentDispatcher.h"

namespace chess {

using cave::DebugId;
using cave::FrameTime;
using cave::IHostServices;

class IChessGameState;

class ChessGameMode final : public cave::IGameMode,
                            public cave::IIntentHandler {
public:
    ChessGameMode(IHostServices& p_host);
    ~ChessGameMode();

    void OnEnter(IHostServices& p_host) final;
    void OnExit(IHostServices& p_host) final;
    void Tick(IHostServices& p_host, const FrameTime& p_time) final;

    bool HandleIntent(cave::Intent& p_intent) override;

    DebugId debugId() const override { return m_debug_id; }

private:
    void CommitStateChange(std::unique_ptr<IChessGameState>&& p_new_state);

    IHostServices& m_host;
    cave::IntentDispatcher& m_intent;
    const DebugId m_debug_id;

    std::unique_ptr<IChessGameState> m_state;
};

}  // namespace chess
