#include "ChessPresenter.h"

#include <cassert>

#include "cave/core/ErrorMacros.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "chess/core/Position.h"

namespace chess {

using namespace ::cave;
using namespace ::chess::core;

ChessPresenter::ChessPresenter(IHostServices& host) noexcept
    : host_(host)
    , board_view_(host)
    , piece_view_(host) {
}

void ChessPresenter::initialize() {
    board_view_.initialize();
    piece_view_.initialize();
}

void ChessPresenter::present() {
    board_view_.drawBoard();
}

void ChessPresenter::redrawBoard(const Position& position) {
    // @TODO: reset tiles?
    piece_view_.redrawBoard(position);
}

static std::pair<Square, Square> GetCastleRookMove(Square from, Square to) {
    if (from == Square::E1) {
        if (to == Square::G1)
            return { Square::H1, Square::F1 };
        if (to == Square::C1)
            return { Square::A1, Square::D1 };
    }
    if (from == Square::E8) {
        if (to == Square::G8)
            return { Square::H8, Square::F8 };
        if (to == Square::C8)
            return { Square::A8, Square::D8 };
    }

    CRASH_NOW_MSG("Invalid castling");
    return {};
}

static Square enpassantCapturedSquare(Move move) {
    return Square::fromFileRank(move.to().file(), move.from().rank());
}

void ChessPresenter::applyMove(const Position& position, Move move) {
    const Square from = move.from();
    const Square to = move.to();
    const Color stm = position.SideToMove();

    if (ecs::Entity captured_piece = piece_view_.entityAt(to); captured_piece.IsValid()) {
        piece_view_.removePiece(to);
    }

    piece_view_.movePiece(from, to);

    switch (move.type()) {
        case MoveType::Normal:
            break;
        case MoveType::Castling: {
            const auto [rook_from, rook_to] = GetCastleRookMove(from, to);
            piece_view_.movePiece(rook_from, rook_to);
        } break;
        case MoveType::Enpassant: {
            piece_view_.removePiece(enpassantCapturedSquare(move));
        } break;
        case MoveType::Promotion: {
            piece_view_.removePiece(to);  // remove pawn
            const PieceType promo_type = move.promo().unwrap();
            const Piece promoted = BuildPiece(promo_type, stm);
            piece_view_.spawnPiece(promoted, to);
        } break;
    }
}

}  // namespace chess
