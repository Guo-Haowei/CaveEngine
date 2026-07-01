#include "ChessGameMode.h"

#include <format>

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/diagnostics/ILogSink.h"
#include "cave/core/error/ErrorMacros.h"
#include "cave/core/typedefs.h"
#include "cave/game/IHostServices.h"

#include "chess/game/ChessGameSession.h"
#include "chess/game/ChessIntent.h"
#include "chess/states/MainMenuState.h"

namespace chess {

using namespace cave;

ChessGameMode::ChessGameMode(IHostServices& p_host)
    : host_(p_host)
    , intent_(p_host.intentDispatcher())
    , debug_id_(MakeDebugId(this)) {
    intent_.addHandler<ChessStateIntent>(this);
}

ChessGameMode::~ChessGameMode() {
    intent_.removeHandler<ChessStateIntent>(this);
}

void ChessGameMode::onEnter(IHostServices& p_host) {
    state_ = std::make_unique<MainMenuState>();
    state_->OnEnter(p_host);
}

void ChessGameMode::onExit(IHostServices& p_host) {
    unused(p_host);
}

void ChessGameMode::tick(IHostServices& p_host, const FrameTime& p_time) {
    if (DEV_VERIFY(state_)) {
        state_->Tick(p_host, p_time);
    }
}

bool ChessGameMode::handleIntent(cave::Intent& p_intent) {
    if (auto intent = dynamic_cast<ChessStateIntent*>(&p_intent)) {
        commitStateChange(std::move(intent->state_));
        return true;
    }

    return false;
}

void ChessGameMode::commitStateChange(std::unique_ptr<IChessGameState>&& new_state) {
    DEV_ASSERT(new_state != nullptr);

    if (state_) {
        state_->OnExit(host_);
    }

    state_ = std::move(new_state);
    state_->OnEnter(host_);
}

}  // namespace chess
