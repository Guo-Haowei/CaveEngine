#include "BoardController.h"

namespace chess {

using namespace ::cave;

BoardController::BoardController() = default;
BoardController::~BoardController() = default;

void BoardController::onCreate(SceneContext& ctx) {
    m_game = std::make_unique<ChessGameMode>(ctx);
    m_game->onEnter(ctx);
}

void BoardController::onDestroy() {
    m_game->onExit();
    m_game.reset();
}

void BoardController::onUpdate(SceneContext& ctx, float dt) {
    m_game->tick(ctx, dt);
}

}  // namespace chess

