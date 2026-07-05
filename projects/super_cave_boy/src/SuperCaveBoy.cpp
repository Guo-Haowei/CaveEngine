#include "SuperCaveBoy.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/game/IHostServices.h"

#include "controllers/BatController.h"
#include "controllers/CameraController.h"
#include "controllers/ExitTrigger.h"
#include "controllers/PlayerController.h"
#include "controllers/SnakeController.h"
#include "controllers/SpiderController.h"

namespace super_cave_boy {

using namespace ::cave;

SuperCaveBoy::SuperCaveBoy() = default;
SuperCaveBoy::~SuperCaveBoy() = default;

void SuperCaveBoy::registerNativeScripts(NativeScriptRegistry& registry) {
    registry.registerScript<CameraController>("CameraController");
    registry.registerScript<PlayerController>("PlayerController");
    registry.registerScript<ExitTrigger>("ExitTrigger");
    registry.registerScript<SpiderController>("SpiderController");
    registry.registerScript<SnakeController>("SnakeController");
    registry.registerScript<BatController>("BatController");
}

void SuperCaveBoy::onGameBegin(IHostServices& host) {
    unused(host);
}

void SuperCaveBoy::onGameEnd(IHostServices& host) {
    unused(host);
}

void SuperCaveBoy::tick(IHostServices& host, const FrameTime& time) {
    unused(host);
    unused(time);
}

}  // namespace super_cave_boy
