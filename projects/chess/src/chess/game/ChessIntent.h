#pragma once
#include "ChessTypes.h"

#include "cave/runtime/intent/Intent.h"

namespace chess {

class MoveIntentBase : public cave::Intent {
public:
    MoveIntentBase(core::Color side, core::Move move) noexcept
        : m_side(side)
        , m_move(move) {}

    core::Color side() const { return m_side; };
    core::Move move() const { return m_move; }

    std::string debugString() const override;

private:
    core::Color m_side;
    core::Move m_move;
};

class ChessMoveIntent : public MoveIntentBase {
public:
    CAVE_DECLARE_INTENT("move.submitted");

    using MoveIntentBase::MoveIntentBase;
};

class AuthMoveCommitted : public MoveIntentBase {
public:
    CAVE_DECLARE_INTENT("move.accepted");

    using MoveIntentBase::MoveIntentBase;

#if USING(DEBUG_BUILD)
    std::string debugString() const override;
#endif
};

class AuthMoveRejected : public MoveIntentBase {
public:
    CAVE_DECLARE_INTENT("move.rejected");

    using MoveIntentBase::MoveIntentBase;

    std::string debugString() const override;
};

class AuthGameOver : public MoveIntentBase {
public:
    CAVE_DECLARE_INTENT("game.over");

    using MoveIntentBase::MoveIntentBase;

    std::string debugString() const override;
};

}  // namespace chess
