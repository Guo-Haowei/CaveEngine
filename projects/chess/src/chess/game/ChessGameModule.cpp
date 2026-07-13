#include "ChessGameModule.h"

#include "chess/game/BoardController.h"
#include "chess/game/MainMenu.h"

namespace chess {

using namespace ::cave;

ChessGameModule::ChessGameModule() = default;
ChessGameModule::~ChessGameModule() = default;

void ChessGameModule::registerNativeScripts(NativeScriptRegistry& registry) {
    registry.registerScript<BoardController>("BoardController");
    registry.registerScript<MainMenu>("MainMenu");
}

}  // namespace chess
