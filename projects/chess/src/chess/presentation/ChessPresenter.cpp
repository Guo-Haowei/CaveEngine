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
static constexpr StringId kCastShadow = "cast_shadow"_sid;

static inline Vector3f SquareToVec(Square p_sq) {
    const auto [file, rank] = p_sq.FileRank();
    return Vector3f{ (float)rank, 0.0f, (float)file };
}

void ChessPresenter::onBoot() {
    highlights_ = {};

    auto& query = host_.sceneQuery();
    selector_ = query.FindFirstEntity("grid_selector");

    // set up tiles
    for (uint8_t i = 0; i < 64; ++i) {
        const char* name = Square(i).ToString();
        tiles_[i] = query.FindFirstEntity(name);
    }

    // set up pieces
    auto add_piece = [&](Piece type, std::string_view name, int count) {
        const uint8_t idx = std::to_underlying(type);
        piece_pools_[idx].reserve(count);
        for (int i = 1; i <= count; ++i) {
            Entity id = query.FindFirstEntity(std::format("{}_{}", name, i));
            piece_pools_[idx].push_back(id);
        }
    };

    add_piece(Piece::WP, "white_pawn", 8);
    add_piece(Piece::WN, "white_knight", 2);
    add_piece(Piece::WB, "white_bishop", 2);
    add_piece(Piece::WR, "white_rook", 2);
    add_piece(Piece::WQ, "white_queen", 1);
    add_piece(Piece::WK, "white_king", 1);
    add_piece(Piece::BP, "black_pawn", 8);
    add_piece(Piece::BN, "black_knight", 2);
    add_piece(Piece::BB, "black_bishop", 2);
    add_piece(Piece::BR, "black_rook", 2);
    add_piece(Piece::BQ, "black_queen", 1);
    add_piece(Piece::BK, "black_king", 1);
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

    Vector3f position = SquareToVec(focused_sq);
    writer.SetProperty(selector_,
                       cave::TransformComponent_Id,
                       kTranslationId,
                       position);
}

void ChessPresenter::redrawBoard(const Position& position) {
    auto& writer = host_.sceneWriter();

    // update pieces
    for (uint8_t p = 0; p < kPieceMax; ++p) {
        const Piece piece = static_cast<Piece>(p);
        const Bitboard bb = position.Bitboard(piece);
        auto& pool = piece_pools_[p];

        int idx = 0;
        for (Square sq : bb.Squares()) {
            // @TODO: properly handle not enough entities in pool caused by promotion
            if (idx >= pool.size()) {
                break;
            }

            Entity e = pool[idx++];
            board_[sq.Index()] = e;

            Vector3f translation = SquareToVec(sq);
            writer.SetProperty(e, TransformComponent_Id, kTranslationId, translation);
            writer.SetProperty(e, MeshRendererComponent_Id, kVisibility, true);
            writer.SetProperty(e, MeshRendererComponent_Id, kCastShadow, true);
        }

        // set the reset of the pieces invisible
        for (; idx < pool.size(); ++idx) {
            Entity e = pool[idx];
            writer.SetProperty(e, MeshRendererComponent_Id, kVisibility, false);
            writer.SetProperty(e, MeshRendererComponent_Id, kCastShadow, false);
        }
    }
}

void ChessPresenter::clearSquare(SceneCommandWriter& writer,
                                 Square square) {
    const Entity e = board_[square.Index()];
    board_[square.Index()] = Entity::Null();

    writer.SetProperty(e, MeshRendererComponent_Id, kVisibility, false);
    writer.SetProperty(e, MeshRendererComponent_Id, kCastShadow, false);
}

void ChessPresenter::movePiece(Entity ent, Square from, Square to) {
    board_[from.Index()] = Entity::Null();
    board_[to.Index()] = ent;

    constexpr auto cid = TransformAnimationComponent_Id;
    auto& writer = host_.sceneWriter();
    writer.AddComponent(ent, cid);
    writer.SetProperty(ent, cid, "begin"_sid, SquareToVec(from));
    writer.SetProperty(ent, cid, "end"_sid, SquareToVec(to));
    writer.SetProperty(ent, cid, "duration"_sid, 0.25f);
    writer.SetProperty(ent, cid, "playing"_sid, true);
    writer.SetProperty(ent, cid, "destroy_on_finish"_sid, true);
}

struct CastleRookMove {
    Square from;
    Square to;
};

CastleRookMove GetCastleRookMove(Square from, Square to) {
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

void ChessPresenter::applyMove(Move move) {
    const Square from = move.From();
    const Square to = move.To();
    Entity src_piece = getEntityAt(from);

    assert(src_piece.IsValid());

    auto& writer = host_.sceneWriter();

    if (Entity captured_piece = getEntityAt(to); captured_piece.IsValid()) {
        clearSquare(writer, to);
    }

    movePiece(src_piece, from, to);

    switch (move.GetType()) {
        case MoveType::Normal:
            break;
        case MoveType::Castling: {
            CastleRookMove rook = GetCastleRookMove(from, to);
            movePiece(board_[rook.from.Index()], rook.from, rook.to);
        } break;
        case MoveType::Enpassant:
            host_.log().Warn(LogChannel::Game, "Handle enpassant");
            break;
        case MoveType::Promotion:
            host_.log().Warn(LogChannel::Game, "Handle promotion");
            break;
    }
}

}  // namespace chess
