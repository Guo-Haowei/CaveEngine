#include "ChessGameModule.h"

#include "cave/core/error/ErrorMacros.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"

#include "chess/core/Bitboard.h"
#include "chess/core/Piece.h"
#include "chess/game/BoardController.h"
#include "chess/game/ChessGameMode.h"

namespace chess {

using namespace ::cave;
using namespace ::cave::literals;
using namespace ::cave::math;
using namespace ::chess::core;
using cave::ecs::Entity;

ChessGameModule::ChessGameModule() = default;
ChessGameModule::~ChessGameModule() = default;

void ChessGameModule::registerNativeScripts(NativeScriptRegistry& registry) {
    registry.registerScript<BoardController>("BoardController");
}

}  // namespace chess
