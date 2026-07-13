#include "ChessIntent.h"

namespace chess {

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
