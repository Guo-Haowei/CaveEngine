#pragma once
#include <cstdint>
#include <format>

#include "chess/core/Move.h"

#include "cave/runtime/intent/Intent.h"

namespace chess {

using PlayerId = core::Color;

enum class ChessEventType : uint8_t {
    SelectionChanged,
    MoveRejected,
    MoveApplied,
    Capture,
    Check,
    PromotionRequested,
    GameOver,
};

}  // namespace chess