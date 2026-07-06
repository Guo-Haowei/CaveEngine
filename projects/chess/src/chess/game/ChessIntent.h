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
    std::string debugString() const override;
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

    ChessMoveIntent(core::Color side, core::Move move) noexcept
        : side_(side)
        , move_(move) {}

    core::Color side() const { return side_; };
    core::Move move() const { return move_; }

#if USING(DEBUG_BUILD)
    std::string debugString() const override;
#endif

private:
    core::Color side_;
    core::Move move_;
};

class AuthMoveCommitted : public ChessMoveIntent {
public:
    CAVE_DECLARE_INTENT("move.accepted");

    using ChessMoveIntent::ChessMoveIntent;

#if USING(DEBUG_BUILD)
    std::string debugString() const override;
#endif
};

class AuthMoveRejected : public ChessMoveIntent {
public:
    CAVE_DECLARE_INTENT("move.rejected");

    using ChessMoveIntent::ChessMoveIntent;

#if USING(DEBUG_BUILD)
    std::string debugString() const override;
#endif
};

class AuthGameOver : public ChessMoveIntent {
public:
    CAVE_DECLARE_INTENT("game.over");

    using ChessMoveIntent::ChessMoveIntent;

#if USING(DEBUG_BUILD)
    std::string debugString() const override;
#endif
};

}  // namespace chess
