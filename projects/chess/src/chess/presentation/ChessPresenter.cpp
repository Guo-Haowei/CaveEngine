#include "ChessPresenter.h"

#include <cassert>

#include "cave/core/ErrorMacros.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "chess/core/Position.h"

namespace chess {

using namespace ::cave;
using namespace ::cave::literals;
using namespace ::cave::math;
using namespace ::chess::core;

static constexpr StringId kTranslationId = "translation"_sid;
static constexpr StringId kVisibility = "visibility"_sid;

ChessPresenter::ChessPresenter(IHostServices& host) noexcept
    : host_(host)
    , piece_view_(host) {
}

// @TODO: refactor this
static inline Vector3f squareToVec(Square square) {
    const auto [file, rank] = square.FileRank();
    return Vector3f{ (float)rank, 0.0f, (float)file };
}

void ChessPresenter::initialize() {
    highlights_ = {};

    auto& query = host_.sceneQuery();
    selector_ = query.findFirstByName("grid_selector");

    // tiles
    for (uint8_t i = 0; i < 64; ++i) {
        const char* name = Square(i).ToString();
        tiles_[i] = query.findFirstByName(name);
    }

    // pieces
    piece_view_.initializePieces();
}

void ChessPresenter::present() {
    auto& writer = host_.sceneWriter();

    for (uint8_t i = 0; i < 64; ++i) {
        const Square sq(i);
        bool visible = highlights_.Test(sq);
        if (sq == focused_sq) {
            visible = false;
        }
        const Entity tile = tiles_[i];
        writer.SetProperty(tile,
                           cave::MeshRendererComponent_Id,
                           kVisibility,
                           visible);
    }

    Vector3f position = squareToVec(focused_sq);
    writer.SetProperty(selector_,
                       cave::TransformComponent_Id,
                       kTranslationId,
                       position);
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

void ChessPresenter::applyMove(const core::Position& position, Move move) {
    const Square from = move.from();
    const Square to = move.to();

    if (Entity captured_piece = piece_view_.entityAt(to); captured_piece.IsValid()) {
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
        case MoveType::Enpassant:
            host_.log().Warn(LogChannel::Game, "Handle enpassant");
            break;
        case MoveType::Promotion: {
            piece_view_.removePiece(to); // remove pawn
            const PieceType promo_type = move.promo().unwrap();
            const Piece promoted = BuildPiece(promo_type, position.SideToMove());
            piece_view_.spawnPiece(promoted, to);
        } break;
    }
}

}  // namespace chess
