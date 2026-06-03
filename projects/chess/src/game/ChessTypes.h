#pragma once
#include <cstdint>
#include <format>

#include "core/Move.h"

#include "cave/runtime/intent/Intent.h"

namespace chess {

using PlayerId = uint8_t;

class MoveIntent : public cave::Intent {
public:
    CAVE_DECLARE_INTENT("chess.move");

    MoveIntent(PlayerId p_player, core::Move p_mv) noexcept
        : player(p_player)
        , mv(p_mv) {}

#if USING(DEBUG_BUILD)
    std::string DebugString() const override {
        return std::format("player={} move={}", player, mv.Uci());
    }
#endif

    const PlayerId player;
    const core::Move mv;
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

}  // namespace chess