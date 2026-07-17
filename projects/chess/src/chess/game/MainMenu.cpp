#include "MainMenu.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/runtime/scene/ISceneTransitionRequests.h"

#include "ChessSpawner.h"

namespace chess {

using namespace ::cave;

constexpr StringId kMainMenuLocal = CAVE_SID("main_menu.local");
constexpr StringId kMainMenuOnline = CAVE_SID("main_menu.online");
constexpr StringId kMainMenuQuit = CAVE_SID("main_menu.quit");

void MainMenu::alwaysRun(cave::SceneCommandWriter& writer) {
    Spawner spawner(SpawnType::MainMenu, query(), writer);
    spawner.spawnPieces();
}

void MainMenu::start() {
    m_local_listener = message().listen(kMainMenuLocal, [this](const Message&) {
        if (DEV_VERIFY(transition())) {
            transition()->requestSceneChange("@res://Gameplay.scene");
        }
    });
    m_online_listener = message().listen(kMainMenuOnline, [this](const Message&) {
        LOG_OK(LogChannel::Game, "main_menu.online received");
    });
    m_quit_listener = message().listen(kMainMenuQuit, [this](const Message&) {
        LOG_OK(LogChannel::Game, "main_menu.quit received");
    });
}

void MainMenu::destroy() {
    message().disconnect(m_local_listener);
    message().disconnect(m_online_listener);
    message().disconnect(m_quit_listener);
}

}  // namespace chess
