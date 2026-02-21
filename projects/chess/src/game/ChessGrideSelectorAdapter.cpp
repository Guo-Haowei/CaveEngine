#include "ChessGrideSelectorAdapter.h"

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
using core::Square;

bool ChessGridSelectorAdapter::CanSelect(int x, int y) {
    const Square sq = Square::FromFileRank((uint8_t)x, (uint8_t)y);

    const core::Position& pos = m_client.Pos();
    return pos.SideToMove() == pos.ColorAt(sq);
}

void ChessGridSelectorAdapter::OnSelect(int x, int y) {
    const Square sq = Square::FromFileRank((uint8_t)x, (uint8_t)y);

    std::span<const Move> moves = m_client.LegalMovesFromSquare(sq);

    core::Bitboard bb;
    for (Move mv : moves) {
        bb.Set(mv.to);
    }
    m_presenter.SetHighlightSquares(bb);
}

bool ChessGridSelectorAdapter::CanDrop(int sx, int sy, int dx, int dy) {
    const Square sq = Square::FromFileRank((uint8_t)sx, (uint8_t)sy);

    std::span<const Move> moves = m_client.LegalMovesFromSquare(sq);
    for (Move mv : moves) {
        const auto [from_file, from_rank] = mv.from.FileRank();
        const auto [to_file, to_rank] = mv.to.FileRank();

        if (from_file == sx && from_rank == sy && to_file == dx && to_rank && dy) {
            return true;
        }
    }

    return false;
}

void ChessGridSelectorAdapter::OnDrop(int sx, int sy, int dx, int dy) {
    m_presenter.SetHighlightSquares({});

    const core::Position& pos = m_client.Pos();
    const PlayerId id = pos.SideToMove() == core::Color::White ? 0 : 1;
    if (m_players[id]) {
        Move move{
            Square::FromFileRank((uint8_t)sx, (uint8_t)sy),
            Square::FromFileRank((uint8_t)dx, (uint8_t)dy),
        };

        auto& inbox = m_players[id]->LocalInbox();
        PlayerIntent intent = {
            IntentType::AttemptMove,
            move,
        };
        inbox.Push(intent);
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
}

void ChessGridSelectorAdapter::Tick(cave::IInputService& p_input) {
    using cave::StringId;
    using cave::math::Vector2i;

    if (p_input.IsActionJustPressed(StringId("ui_right"))) {
        m_controller->MoveFocus(Vector2i(1, 0));
    }
    if (p_input.IsActionJustPressed(StringId("ui_left"))) {
        m_controller->MoveFocus(Vector2i(-1, 0));
    }
    if (p_input.IsActionJustPressed(StringId("ui_up"))) {
        m_controller->MoveFocus(Vector2i(0, 1));
    }
    if (p_input.IsActionJustPressed(StringId("ui_down"))) {
        m_controller->MoveFocus(Vector2i(0, -1));
    }
    if (p_input.IsActionJustPressed(StringId("ui_accept"))) {
        m_controller->Confirm();
    }
    if (p_input.IsActionJustPressed(StringId("ui_back"))) {
        m_controller->Cancel();
    }
}

}  // namespace chess
