#include "ChessPieceView.h"

#include "cave/core/error/ErrorMacros.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/ecs/components/TransformAnimationComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

#include "chess/presentation/ChessUtils.h"

namespace chess {

using namespace ::cave;
using namespace ::chess::core;

static std::pair<Square, Square> castleRookMove(Square from, Square to) {
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

ecs::Entity ChessPieceView::Entry::getAndAdvance() {
    return pool[cursor++];
}

void ChessPieceView::initialize() {
    auto add_piece = [&](Piece type, std::string_view name) {
        const uint8_t idx = std::to_underlying(type);

        Entry& entry = m_piece_pool[idx];

        for (int i = 1;; ++i) {
            Entity id = m_query.findFirstByName(std::format("{}_{}", name, i));
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

void ChessPieceView::redrawPieces(const Position& position) {
    for (uint8_t p = 0; p < kPieceMax; ++p) {
        const Piece piece = static_cast<Piece>(p);
        const Bitboard bb = position.Bitboard(piece);

        auto& entry = m_piece_pool[p];

        for (Square sq : bb.Squares()) {
            spawnPiece(piece, sq);
        }

        // set the reset of the pieces invisible
        for (uint8_t i = entry.cursor; i < entry.pool.size(); ++i) {
            Entity e = entry.pool[i];
            auto renderer = m_query.component<MeshRendererComponent>(e);
            if (DEV_VERIFY(renderer)) {
                renderer->SetVisible(false);
                renderer->SetCastShadow(false);
            }
        }
    }
}

void ChessPieceView::spawnPiece(Piece piece, Square square) {
    auto& entry = m_piece_pool[std::to_underlying(piece)];
    DEV_ASSERT(entry.cursor < entry.pool.size());
    Entity ent = entry.getAndAdvance();

    Vec3f translation = squareToVec(square);

    auto renderer = m_query.component<MeshRendererComponent>(ent);
    if (DEV_VERIFY(renderer)) {
        renderer->SetVisible(true);
        renderer->SetCastShadow(true);
    }
    auto transform = m_query.component<TransformComponent>(ent);
    if (DEV_VERIFY(transform)) {
        transform->setTranslation(translation);
    }

    m_board[square.index()] = ent;
}

void ChessPieceView::removePiece(Square square) {
    const Entity ent = m_board[square.index()];
    m_board[square.index()] = Entity::Null();

    auto renderer = m_query.component<MeshRendererComponent>(ent);
    if (DEV_VERIFY(renderer)) {
        renderer->SetVisible(false);
        renderer->SetCastShadow(false);
    }
}

void ChessPieceView::movePiece(Square from, Square to) {
    Entity ent = m_board[from.index()];
    DEV_ASSERT(ent.IsValid());

    m_board[from.index()] = Entity::Null();
    m_board[to.index()] = ent;

    auto anim = m_query.addComponent<TransformAnimationComponent>(ent);
    if (DEV_VERIFY(anim)) {
        anim->begin = squareToVec(from);
        anim->end = squareToVec(to);
        anim->duration = 0.25f;
        anim->playing = true;
        anim->destroy_on_finish = true;
    }
}

void ChessPieceView::applyMove(const Position& position, Move move) {
    const Square from = move.from();
    const Square to = move.to();
    const Color stm = position.sideToMove();

    if (ecs::Entity captured_piece = entityAt(to); captured_piece.IsValid()) {
        removePiece(to);
    }

    movePiece(from, to);

    switch (move.type()) {
        case MoveType::Normal:
            break;
        case MoveType::Castling: {
            const auto [rook_from, rook_to] = castleRookMove(from, to);
            movePiece(rook_from, rook_to);
        } break;
        case MoveType::Enpassant: {
            removePiece(EnpassantCapturedSquare(from, to));
        } break;
        case MoveType::Promotion: {
            removePiece(to);  // remove pawn
            const PieceType promo_type = move.promo().unwrap();
            const Piece promoted = BuildPiece(promo_type, stm);
            spawnPiece(promoted, to);
        } break;
    }
}

}  // namespace chess
