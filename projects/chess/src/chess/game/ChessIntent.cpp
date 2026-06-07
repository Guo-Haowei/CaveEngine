#include "ChessIntent.h"

#include "chess/states/IChessGameState.h"

namespace chess {

ChessStateIntent::ChessStateIntent(std::unique_ptr<IChessGameState> state)
    : state_(std::move(state))
#if USING(DEBUG_BUILD)
    , debug_name_(state_->DebugName())
#endif
{
}

ChessStateIntent::~ChessStateIntent() = default;

#if USING(DEBUG_BUILD)
std::string ChessStateIntent::DebugString() const {
    return std::format("->{}", debug_name_);
}

std::string ChessMoveIntent::DebugString() const {
    return std::format("p={} mv={}", std::to_underlying(player_), mv_.Uci());
}

std::string AuthMoveCommitted::DebugString() const {
    return "";
}

std::string AuthMoveRejected::DebugString() const {
    return "";
}

std::string AuthGameOver::DebugString() const {
    return "";
}
#endif

}  // namespace chess
