#include "GameModule.h"

#include "cave/core/diagnostics/Log.h"

#include "PlayerController.h"

using namespace ::cave;

namespace super_cave_boy {

GameModule::GameModule()
    : controller_{ std::make_unique<PlayerController>() } {
}

GameModule::~GameModule() = default;

void GameModule::onModuleLoaded(IHostServices& host) {
    LOG_OK(LogChannel::Game, "GameModule Loaded");

    unused(host);
}

void GameModule::onModuleUnloaded(IHostServices&) {
}

void GameModule::onGameBegin(IHostServices& host) {
    controller_->onCreate(host);
}

void GameModule::onGameEnd(IHostServices& host) {
    controller_->onDestroy(host);
}

void GameModule::tick(IHostServices& host, const FrameTime& time) {
    controller_->onUpdate(host, time);
}

}  // namespace super_cave_boy
