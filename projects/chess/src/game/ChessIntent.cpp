#include "ChessIntent.h"

#include "IChessGameState.h"

namespace chess {

ChessStateIntent::ChessStateIntent(std::unique_ptr<IChessGameState> p_state)
    : state(std::move(p_state)) {}

ChessStateIntent::~ChessStateIntent() = default;

#if USING(DEBUG_BUILD)
std::string ChessStateIntent::DebugString() const {
    return std::format("->{}", state->DebugName());
}
#endif

#if USING(DEBUG_BUILD)
std::string ChessMoveIntent::DebugString() const {
    return std::format("player={} move={}", player, mv.Uci());
}
#endif

}  // namespace chess
