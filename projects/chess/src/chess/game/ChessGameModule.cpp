#include "ChessGameModule.h"

#include "chess/game/BoardController.h"

namespace chess {

using namespace ::cave;

ChessGameModule::ChessGameModule() = default;
ChessGameModule::~ChessGameModule() = default;

void ChessGameModule::registerNativeScripts(NativeScriptRegistry& registry) {
    registry.registerScript<BoardController>("BoardController");
}

}  // namespace chess
