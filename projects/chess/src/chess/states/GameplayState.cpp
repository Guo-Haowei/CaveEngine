#include "GameplayState.h"

#include "chess/game/ChessGameSession.h"

namespace chess {

GameplayState::GameplayState() noexcept = default;

GameplayState::~GameplayState() = default;

void GameplayState::OnEnter(cave::IHostServices& p_host) {
    session_ = std::make_unique<ChessGameSession>(p_host);
    session_->onEnterBoot();
}

void GameplayState::OnExit(cave::IHostServices& p_host) {
    unused(p_host);

    session_.reset();
}

void GameplayState::Tick(cave::IHostServices&, const cave::FrameTime&) {

    session_->tick();
}

}  // namespace chess
