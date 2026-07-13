#include "MainMenu.h"

#include "ChessSpawner.h"

namespace chess {

void MainMenu::alwaysRun(cave::SceneCommandWriter& writer) {
    Spawner spawner(SpawnType::MainMenu, query(), writer);
    spawner.spawnPieces();
}

}  // namespace chess
