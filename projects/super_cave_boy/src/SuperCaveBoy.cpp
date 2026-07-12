#include "SuperCaveBoy.h"

#include "cave/core/diagnostics/Log.h"

#include "controllers/BatController.h"
#include "controllers/CameraController.h"
#include "controllers/ExitController.h"
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
    registry.registerScript<ExitController>("ExitController");
    registry.registerScript<SpiderController>("SpiderController");
    registry.registerScript<SnakeController>("SnakeController");
    registry.registerScript<BatController>("BatController");
}

}  // namespace super_cave_boy
