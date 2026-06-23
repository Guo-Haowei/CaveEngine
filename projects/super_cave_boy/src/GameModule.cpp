#include "GameModule.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/game/IHostServices.h"

#include "controllers/BatController.h"
#include "controllers/CameraController.h"
#include "controllers/PlayerController.h"
#include "controllers/SnakeController.h"
#include "controllers/SpiderController.h"

namespace super_cave_boy {

using namespace ::cave;

GameModule::GameModule() = default;
GameModule::~GameModule() = default;

void GameModule::registerNativeScripts(NativeScriptRegistry& registry) {
    registry.registerScript<CameraController>("CameraController");
    registry.registerScript<PlayerController>("PlayerController");
    registry.registerScript<SpiderController>("SpiderController");
    registry.registerScript<SnakeController>("SnakeController");
    registry.registerScript<BatController>("BatController");
}

void GameModule::onModuleLoaded(IHostServices& host) {
    LOG_OK(LogChannel::Game, "GameModule Loaded");

    unused(host);
}

void GameModule::onModuleUnloaded(IHostServices&) {
}

void GameModule::onGameBegin(IHostServices& host) {
    unused(host);
}

void GameModule::onGameEnd(IHostServices& host) {
    unused(host);
}

void GameModule::tick(IHostServices& host, const FrameTime& time) {
    unused(host);
    unused(time);
}

}  // namespace super_cave_boy
