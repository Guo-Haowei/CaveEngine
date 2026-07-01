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
    ChessGameMode(IHostServices& host);
    ~ChessGameMode();

    void onEnter(IHostServices& host) final;
    void onExit(IHostServices& host) final;
    void tick(IHostServices& host, const FrameTime& time) final;

    bool handleIntent(cave::Intent& intent) override;

    DebugId debugId() const override { return debug_id_; }

private:
    void commitStateChange(std::unique_ptr<IChessGameState>&& new_state);

    IHostServices& host_;
    cave::IntentDispatcher& intent_;
    const DebugId debug_id_;

    std::unique_ptr<IChessGameState> state_;
};

}  // namespace chess
