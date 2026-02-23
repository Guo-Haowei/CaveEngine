#pragma once
#include <cstdint>
#include "core/Move.h"

namespace chess {

enum class IntentType : uint8_t {
    AttemptMove,
    OfferDraw,
    Resign,
};

struct PlayerIntent {
    IntentType type{};
    core::Move move;
};

enum class ChessEventType : uint8_t {
    SelectionChanged,
    MoveRejected,
    MoveApplied,
    Capture,
    Check,
    PromotionRequested,
    GameOver,
};

struct ChessEvent {
    ChessEventType type{};
    // include Move, captured piece, result, etc.
};

}  // namespace chess