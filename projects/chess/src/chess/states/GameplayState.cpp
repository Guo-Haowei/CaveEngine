#include "GameplayState.h"

#include "chess/game/ChessGameSession.h"

namespace chess {

using namespace ::cave;

GameplayState::GameplayState() noexcept = default;

GameplayState::~GameplayState() = default;

void GameplayState::onEnter(SceneContext& ctx) {
    session_ = std::make_unique<ChessGameSession>();
    session_->onEnterBoot(ctx);
}

void GameplayState::onExit() {
    session_.reset();
}

void GameplayState::tick(SceneContext& ctx, float) {
    session_->tick(ctx);
}

}  // namespace chess
