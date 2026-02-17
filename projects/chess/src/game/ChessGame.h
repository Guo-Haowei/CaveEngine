#pragma once
#include <cstdint>

namespace chess {

enum class ChessIntentType : uint8_t {
    SelectSquare,
    AttemptMove,
    CancelSelection,
    OfferDraw,
    Resign,
};

struct ChessIntent {
    ChessIntentType type{};
    uint8_t from_file{}, from_rank{};
    uint8_t to_file{}, to_rank{};
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

class ChessGame {
public:
private:
};

}  // namespace chess