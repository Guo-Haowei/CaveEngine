#include "BoardController.h"

#include "ChessSpawner.h"

namespace chess {

BoardController::BoardController() = default;
BoardController::~BoardController() = default;

void BoardController::alwaysRun(cave::SceneCommandWriter& writer) {
    Spawner spawner(SpawnType::Gameplay, query(), writer);
    spawner.spawnPieces();
}

void BoardController::start() {
    m_game = MakeOwner<ChessGameMode>(runtime(), m_intent_bus);
    m_game->onEnter();
}

void BoardController::destroy() {
    m_game->onExit();
    m_game.reset();
}

void BoardController::update(float dt) {
    m_game->tick(dt);
}

}  // namespace chess
