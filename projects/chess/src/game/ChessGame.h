#pragma once
#include <cstdint>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include "core/Position.h"
#include "core/Move.h"

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
    void ResetBoard();

    std::span<const core::Move> LegalMoves() const { return m_moves; }

    std::span<const core::Move> LegalMovesFromSquare(core::Square p_sq);

private:
    void OnPositionChange();

    core::Position m_pos;

    core::MoveList m_moves;
    std::unordered_map<core::Square, std::vector<core::Move>> m_move_cache;
};

}  // namespace chess