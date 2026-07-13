#include "MainMenu.h"

#include "cave/runtime/scene/ISceneTransitionRequests.h"

#include "ChessSpawner.h"

namespace chess {

using namespace ::cave;
using namespace ::cave::literals;

constexpr StringId kMenuStart = "main_menu.start"_sid;

void MainMenu::alwaysRun(cave::SceneCommandWriter& writer) {
    Spawner spawner(SpawnType::MainMenu, query(), writer);
    spawner.spawnPieces();
}

void MainMenu::start() {
    m_start_listener = message().listen(kMenuStart, [this](const Message&) {
        if (DEV_VERIFY(transition())) {
            transition()->requestSceneChange("@res://Gameplay.scene");
        }
    });
}

void MainMenu::destroy() {
    message().disconnect(m_start_listener);
}

}  // namespace chess
