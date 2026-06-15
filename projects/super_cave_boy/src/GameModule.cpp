#include "GameModule.h"

#include "cave/core/error/ErrorMacros.h"
#include "cave/core/diagnostics/Log.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"

using namespace ::cave;
using namespace ::cave::literals;
using namespace ::cave::math;
using cave::ecs::Entity;

GameModule::GameModule() = default;
GameModule::~GameModule() = default;

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
