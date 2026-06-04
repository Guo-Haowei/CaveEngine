#include "ChessIntent.h"

#include "IChessGameState.h"

namespace chess {

ChessStateIntent::ChessStateIntent(std::unique_ptr<IChessGameState> p_state)
    : state(std::move(p_state))
#if USING(DEBUG_BUILD)
    , m_debug_name(state->DebugName())
#endif
{
}

ChessStateIntent::~ChessStateIntent() = default;

#if USING(DEBUG_BUILD)
std::string ChessStateIntent::DebugString() const {
    return std::format("->{}", m_debug_name);
}

std::string ChessMoveIntent::DebugString() const {
    return std::format("p={} mv={}", player, mv.Uci());
}

std::string AuthMoveCommitted::DebugString() const {
    return "Commited";
}

std::string AuthMoveRejected::DebugString() const {
    return "Rejected";
}

std::string AuthGameOver::DebugString() const {
    return "AuthGameOver";
}
#endif

}  // namespace chess
