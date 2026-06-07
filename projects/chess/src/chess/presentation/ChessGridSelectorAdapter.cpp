#include "ChessGridSelectorAdapter.h"

#include "cave/game/IHostServices.h"
#include "cave/runtime/controller/GridSelectController.h"
#include "cave/runtime/input/IGameInput.h"

#include "chess/agents/LocalHumanAgent.h"
#include "chess/game/ChessGameClient.h"
#include "chess/game/ChessIntent.h"
#include "chess/game/ChessMatchAuthority.h"
#include "chess/presentation/ChessPresenter.h"

namespace chess {

using namespace cave::literals;
using core::Color;
using core::Move;
using core::MoveType;
using core::Piece;
using core::Square;

bool ChessGridSelectorAdapter::canSelect(int x, int y) {
    const Square sq = Square::FromFileRank((uint8_t)x, (uint8_t)y);
    std::span<const Move> moves = client_.LegalMovesFromSquare(sq);

    return !moves.empty();
}

void ChessGridSelectorAdapter::onSelect(int x, int y) {
    const Square sq = Square::FromFileRank((uint8_t)x, (uint8_t)y);
    std::span<const Move> moves = client_.LegalMovesFromSquare(sq);

    core::Bitboard bb;
    for (Move mv : moves) {
        bb.Set(mv.To());
    }
    presenter_.SetHighlightSquares(bb);
}

bool ChessGridSelectorAdapter::canDrop(int sx, int sy, int dx, int dy) {
    const Square sq = Square::FromFileRank((uint8_t)sx, (uint8_t)sy);

    std::span<const Move> moves = client_.LegalMovesFromSquare(sq);
    for (Move mv : moves) {
        const auto [from_file, from_rank] = mv.From().FileRank();
        const auto [to_file, to_rank] = mv.To().FileRank();

        if (from_file == sx && from_rank == sy && to_file == dx && to_rank == dy) {
            return true;
        }
    }

    return false;
}

void ChessGridSelectorAdapter::onDrop(int sx, int sy, int dx, int dy) {
    presenter_.SetHighlightSquares({});

    const core::Position& pos = client_.Replica();
    const PlayerId id = pos.SideToMove() == core::Color::White ? 0 : 1;

    if (LocalHumanAgent* agent = get_player_cb_(id)) {
        const Square from = Square::FromFileRank((uint8_t)sx, (uint8_t)sy);
        const Square to = Square::FromFileRank((uint8_t)dx, (uint8_t)dy);

        std::span<const Move> moves = client_.LegalMovesFromSquare(from);
        Move move = Move::Null();
        for (Move mv : moves) {
            if (mv.To() == to) {
                move = mv;
                break;
            }
        }
        assert(move.IsValid());

        intent_.Queue<ChessMoveIntent>(id, move);
    }
}

void ChessGridSelectorAdapter::onCancel() {
    presenter_.SetHighlightSquares({});
}

void ChessGridSelectorAdapter::onInvalid(int sx, int sy, int dx, int dy) {
    (void)sx;
    (void)sy;
    (void)dx;
    (void)dy;
    LOG_ERROR("can't select/drop");
}

void ChessGridSelectorAdapter::tickPointer(const cave::IGameInput& input) {
}

void ChessGridSelectorAdapter::tickKeyboard(const cave::IGameInput& input) {

    // @TODO: player
    // @TODO: consume action instead
    if (input.isJustPressed("ui_right"_sid)) {
        controller_->MoveFocus(1, 0);
    }
    if (input.isJustPressed("ui_left"_sid)) {
        controller_->MoveFocus(-1, 0);
    }
    if (input.isJustPressed("ui_up"_sid)) {
        controller_->MoveFocus(0, 1);
    }
    if (input.isJustPressed("ui_down"_sid)) {
        controller_->MoveFocus(0, -1);
    }
    if (input.isJustPressed("ui_accept"_sid)) {
        controller_->Confirm();
    }
    if (input.isJustPressed("ui_back"_sid)) {
        controller_->Cancel();
    }

    const float dx = input.getStrength("ui_axis_x"_sid);
    const float dy = input.getStrength("ui_axis_y"_sid);
    if (dx > 0.5f) {
        controller_->MoveFocus(1, 0);
    } else if (dx < -0.5f) {
        controller_->MoveFocus(-1, 0);
    }

    if (dy > 0.5f) {
        controller_->MoveFocus(0, 1);
    } else if (dy < -0.5f) {
        controller_->MoveFocus(0, -1);
    }
}

void ChessGridSelectorAdapter::tick(const cave::IGameInput& input) {
    tickPointer(input);
    tickKeyboard(input);
}

}  // namespace chess
