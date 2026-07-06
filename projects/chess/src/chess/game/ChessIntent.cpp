#include "ChessIntent.h"

#include "chess/states/IChessGameState.h"

namespace chess {

ChessStateIntent::ChessStateIntent(std::unique_ptr<IChessGameState> state)
    : state_(std::move(state))
#if USING(DEBUG_BUILD)
    , debug_name_(state_->debugName())
#endif
{
}

ChessStateIntent::~ChessStateIntent() = default;

#if USING(DEBUG_BUILD)
std::string ChessStateIntent::debugString() const {
    return std::format("->{}", debug_name_);
}

std::string ChessMoveIntent::debugString() const {
    return std::format("p={} mv={}", std::to_underlying(side_), move_.uci());
}

std::string AuthMoveCommitted::debugString() const {
    return "";
}

std::string AuthMoveRejected::debugString() const {
    return "";
}

std::string AuthGameOver::debugString() const {
    return "";
}
#endif

}  // namespace chess
