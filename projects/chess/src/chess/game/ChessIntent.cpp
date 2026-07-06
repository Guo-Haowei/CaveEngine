#include "ChessIntent.h"

#include "chess/states/IChessGameState.h"

namespace chess {

ChessStateIntent::ChessStateIntent(std::unique_ptr<IChessGameState> state)
    : m_state(std::move(state))
    , m_debug_name(m_state->debugName()) {
}

ChessStateIntent::~ChessStateIntent() = default;

std::string ChessStateIntent::debugString() const {
    return std::format("->{}", m_debug_name);
}

std::string MoveIntentBase::debugString() const {
    return std::format("p={} mv={}", std::to_underlying(m_side), m_move.uci());
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

}  // namespace chess
