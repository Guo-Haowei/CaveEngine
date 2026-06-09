#include "ChessPieceRegistry.h"

#include "cave/core/ErrorMacros.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace chess {

using namespace ::cave;
using namespace ::cave::literals;
using namespace ::cave::math;
using namespace ::chess::core;

static constexpr StringId kTranslationId = "translation"_sid;
static constexpr StringId kVisibility = "visibility"_sid;
static constexpr StringId kCastShadow = "cast_shadow"_sid;

// @TODO: refactor this
static inline Vector3f squareToVec(Square square) {
    const auto [file, rank] = square.FileRank();
    return Vector3f{ (float)rank, 0.0f, (float)file };
}

ecs::Entity ChessPieceView::Entry::getAndAdvance() {
    return pool[cursor++];
}

ChessPieceView::ChessPieceView(cave::IHostServices& host) noexcept
    : host_(host)
    , writer_(host.sceneWriter()) {
}

void ChessPieceView::initializePieces() {
    auto& query = host_.sceneQuery();

    auto add_piece = [&](Piece type, std::string_view name) {
        const uint8_t idx = std::to_underlying(type);

        Entry& entry = piece_pool_[idx];

        for (int i = 1;; ++i) {
            Entity id = query.findFirstByName(std::format("{}_{}", name, i));
            if (!id.IsValid()) {
                break;
            }
            entry.pool.emplace_back(id);
        }
    };

    add_piece(Piece::WP, "white_pawn");
    add_piece(Piece::WN, "white_knight");
    add_piece(Piece::WB, "white_bishop");
    add_piece(Piece::WR, "white_rook");
    add_piece(Piece::WQ, "white_queen");
    add_piece(Piece::WK, "white_king");
    add_piece(Piece::BP, "black_pawn");
    add_piece(Piece::BN, "black_knight");
    add_piece(Piece::BB, "black_bishop");
    add_piece(Piece::BR, "black_rook");
    add_piece(Piece::BQ, "black_queen");
    add_piece(Piece::BK, "black_king");
}

void ChessPieceView::redrawBoard(const Position& position) {
    for (uint8_t p = 0; p < kPieceMax; ++p) {
        const Piece piece = static_cast<Piece>(p);
        const Bitboard bb = position.Bitboard(piece);

        auto& entry = piece_pool_[p];

        for (Square sq : bb.Squares()) {
            spawnPiece(piece, sq);
        }

        // set the reset of the pieces invisible
        for (uint8_t i = entry.cursor; i < entry.pool.size(); ++i) {
            Entity e = entry.pool[i];
            writer_.SetProperty(e, MeshRendererComponent_Id, kVisibility, false);
            writer_.SetProperty(e, MeshRendererComponent_Id, kCastShadow, false);
        }
    }
}

void ChessPieceView::spawnPiece(Piece piece, Square square) {
    auto& entry = piece_pool_[std::to_underlying(piece)];
    DEV_ASSERT(entry.cursor < entry.pool.size());
    Entity ent = entry.getAndAdvance();

    Vector3f translation = squareToVec(square);

    writer_.SetProperty(ent, TransformComponent_Id, kTranslationId, translation);
    writer_.SetProperty(ent, MeshRendererComponent_Id, kVisibility, true);
    writer_.SetProperty(ent, MeshRendererComponent_Id, kCastShadow, true);

    board_[square.Index()] = ent;
}

void ChessPieceView::removePiece(core::Square square) {
    const Entity e = board_[square.Index()];
    board_[square.Index()] = Entity::Null();

    writer_.SetProperty(e, MeshRendererComponent_Id, kVisibility, false);
    writer_.SetProperty(e, MeshRendererComponent_Id, kCastShadow, false);
}

void ChessPieceView::movePiece(Square from, Square to) {
    Entity ent = board_[from.Index()];
    DEV_ASSERT(ent.IsValid());

    board_[from.Index()] = Entity::Null();
    board_[to.Index()] = ent;

    constexpr auto cid = TransformAnimationComponent_Id;
    auto& writer = host_.sceneWriter();
    writer.AddComponent(ent, cid);
    writer.SetProperty(ent, cid, "begin"_sid, squareToVec(from));
    writer.SetProperty(ent, cid, "end"_sid, squareToVec(to));
    writer.SetProperty(ent, cid, "duration"_sid, 0.25f);
    writer.SetProperty(ent, cid, "playing"_sid, true);
    writer.SetProperty(ent, cid, "destroy_on_finish"_sid, true);
}

}  // namespace chess
