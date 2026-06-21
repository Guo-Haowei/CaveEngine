#include "GameModule.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/game/IHostServices.h"

#include "CameraController.h"
#include "PlayerController.h"
#include "SnakeController.h"

namespace super_cave_boy {

using namespace ::cave;

GameModule::GameModule() = default;
GameModule::~GameModule() = default;

void GameModule::registerNativeScripts(NativeScriptRegistry& registry) {
    registry.registerScript<PlayerController>("PlayerController");
    registry.registerScript<SnakeController>("SnakeController");
    registry.registerScript<CameraController>("CameraController");
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
