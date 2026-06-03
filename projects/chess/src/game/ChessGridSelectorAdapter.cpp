#include "ChessGridSelectorAdapter.h"

#include "cave/game/IHostServices.h"
#include "cave/runtime/controller/GridSelectController.h"
#include "cave/runtime/framework/IInputService.h"

#include "ChessGameClient.h"
#include "ChessMatchAuthority.h"
#include "ChessPresenter.h"
#include "LocalHumanAgent.h"

namespace chess {

using core::Color;
using core::Move;
using core::MoveType;
using core::Piece;
using core::Square;

bool ChessGridSelectorAdapter::CanSelect(int x, int y) {
    const Square sq = Square::FromFileRank((uint8_t)x, (uint8_t)y);
    std::span<const Move> moves = m_client.LegalMovesFromSquare(sq);

    return !moves.empty();
}

void ChessGridSelectorAdapter::OnSelect(int x, int y) {
    const Square sq = Square::FromFileRank((uint8_t)x, (uint8_t)y);
    std::span<const Move> moves = m_client.LegalMovesFromSquare(sq);

    core::Bitboard bb;
    for (Move mv : moves) {
        bb.Set(mv.To());
    }
    m_presenter.SetHighlightSquares(bb);
}

bool ChessGridSelectorAdapter::CanDrop(int sx, int sy, int dx, int dy) {
    const Square sq = Square::FromFileRank((uint8_t)sx, (uint8_t)sy);

    std::span<const Move> moves = m_client.LegalMovesFromSquare(sq);
    for (Move mv : moves) {
        const auto [from_file, from_rank] = mv.From().FileRank();
        const auto [to_file, to_rank] = mv.To().FileRank();

        if (from_file == sx && from_rank == sy && to_file == dx && to_rank == dy) {
            return true;
        }
    }

    return false;
}

void ChessGridSelectorAdapter::OnDrop(int sx, int sy, int dx, int dy) {
    m_presenter.SetHighlightSquares({});

    const core::Position& pos = m_client.Replica();
    const PlayerId id = pos.SideToMove() == core::Color::White ? 0 : 1;

    if (LocalHumanAgent* agent = m_get_player_func(id)) {
        const Square from = Square::FromFileRank((uint8_t)sx, (uint8_t)sy);
        const Square to = Square::FromFileRank((uint8_t)dx, (uint8_t)dy);

        std::span<const Move> moves = m_client.LegalMovesFromSquare(from);
        Move move = Move::Null();
        for (Move mv : moves) {
            if (mv.To() == to) {
                move = mv;
                break;
            }
        }
        assert(move.IsValid());

        m_intent.PushIntent<MoveIntent>(id, move);
    }
}

void ChessGridSelectorAdapter::OnCancel() {
    m_presenter.SetHighlightSquares({});
}

void ChessGridSelectorAdapter::OnInvalid(int sx, int sy, int dx, int dy) {
    (void)sx;
    (void)sy;
    (void)dx;
    (void)dy;
    LOG_ERROR("can't select/drop");
}

void ChessGridSelectorAdapter::Tick(cave::IInputService& p_input) {
    using cave::StringId;
    using cave::math::Vector2i;

    // @TODO: player
    if (p_input.IsActionJustPressed(StringId("ui_right"))) {
        m_controller->MoveFocus(1, 0);
    }
    if (p_input.IsActionJustPressed(StringId("ui_left"))) {
        m_controller->MoveFocus(-1, 0);
    }
    if (p_input.IsActionJustPressed(StringId("ui_up"))) {
        m_controller->MoveFocus(0, 1);
    }
    if (p_input.IsActionJustPressed(StringId("ui_down"))) {
        m_controller->MoveFocus(0, -1);
    }
    if (p_input.IsActionJustPressed(StringId("ui_accept"))) {
        m_controller->Confirm();
    }
    if (p_input.IsActionJustPressed(StringId("ui_back"))) {
        m_controller->Cancel();
    }

    const float dx = p_input.GetActionStrength(StringId("ui_axis_x"));
    const float dy = p_input.GetActionStrength(StringId("ui_axis_y"));
    if (dx > 0.5f) {
        m_controller->MoveFocus(1, 0);
    } else if (dx < -0.5f) {
        m_controller->MoveFocus(-1, 0);
    }

    if (dy > 0.5f) {
        m_controller->MoveFocus(0, 1);
    } else if (dy < -0.5f) {
        m_controller->MoveFocus(0, -1);
    }
}

}  // namespace chess
