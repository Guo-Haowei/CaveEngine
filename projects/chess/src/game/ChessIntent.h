#pragma once
#include "ChessTypes.h"

#include "cave/runtime/intent/Intent.h"

namespace chess {

class IChessGameState;

class ChessStateIntent : public cave::Intent {
public:
    CAVE_DECLARE_INTENT("chess.state");

    ChessStateIntent(std::unique_ptr<IChessGameState> p_state);
    ~ChessStateIntent();

#if USING(DEBUG_BUILD)
    std::string DebugString() const override;
#endif

    std::unique_ptr<IChessGameState> state;

private:
#if USING(DEBUG_BUILD)
    const std::string m_debug_name;
#endif
};

class ChessMoveIntent : public cave::Intent {
public:
    CAVE_DECLARE_INTENT("move.submitted");

    ChessMoveIntent(PlayerId p_player, core::Move p_mv) noexcept
        : player(p_player)
        , mv(p_mv) {}

#if USING(DEBUG_BUILD)
    std::string DebugString() const override;
#endif

    const PlayerId player;
    const core::Move mv;
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
