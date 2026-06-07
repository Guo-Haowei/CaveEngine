#pragma once
#include "ChessTypes.h"

#include "cave/runtime/intent/Intent.h"

namespace chess {

class IChessGameState;

class ChessStateIntent : public cave::Intent {
public:
    CAVE_DECLARE_INTENT("chess.state");

    ChessStateIntent(std::unique_ptr<IChessGameState> state);
    ~ChessStateIntent();

#if USING(DEBUG_BUILD)
    std::string DebugString() const override;
#endif

    std::unique_ptr<IChessGameState> state_;

private:
#if USING(DEBUG_BUILD)
    const std::string debug_name_;
#endif
};

class ChessMoveIntent : public cave::Intent {
public:
    CAVE_DECLARE_INTENT("move.submitted");

    ChessMoveIntent(PlayerId player, core::Move mv) noexcept
        : player_(player)
        , mv_(mv) {}

    PlayerId player() const { return player_; };
    core::Move move() const { return mv_; }

#if USING(DEBUG_BUILD)
    std::string DebugString() const override;
#endif

private:
    PlayerId player_;
    core::Move mv_;
};

class AuthMoveCommitted : public ChessMoveIntent {
public:
    CAVE_DECLARE_INTENT("move.accepted");

    using ChessMoveIntent::ChessMoveIntent;

#if USING(DEBUG_BUILD)
    std::string DebugString() const override;
#endif
};

class AuthMoveRejected : public ChessMoveIntent {
public:
    CAVE_DECLARE_INTENT("move.rejected");

    using ChessMoveIntent::ChessMoveIntent;

#if USING(DEBUG_BUILD)
    std::string DebugString() const override;
#endif
};

class AuthGameOver : public ChessMoveIntent {
public:
    CAVE_DECLARE_INTENT("game.over");

    using ChessMoveIntent::ChessMoveIntent;

#if USING(DEBUG_BUILD)
    std::string DebugString() const override;
#endif
};


}  // namespace chess
