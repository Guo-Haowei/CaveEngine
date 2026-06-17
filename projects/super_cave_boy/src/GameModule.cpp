#include "GameModule.h"

#include "cave/core/diagnostics/Log.h"

#include "CameraController.h"
#include "PlayerController.h"

using namespace ::cave;

namespace super_cave_boy {

GameModule::GameModule() = default;
GameModule::~GameModule() = default;

void GameModule::registerNativeScripts(NativeScriptRegistry& registry) {
    registry.registerScript<PlayerController>("PlayerController");
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
