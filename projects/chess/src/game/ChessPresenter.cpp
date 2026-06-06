#include "ChessPresenter.h"

#include <cassert>

#include "cave/core/ErrorMacros.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "core/Position.h"

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

void ChessPresenter::OnBoot(cave::SceneQuery& p_query) {
    m_highlights = {};

    m_selector = p_query.FindFirstEntity("grid_selector");

    // set up tiles
    for (uint8_t i = 0; i < 64; ++i) {
        const char* name = Square(i).ToString();
        m_tiles[i] = p_query.FindFirstEntity(name);
    }

    // set up pieces
    auto add_piece = [&](Piece p_type, std::string_view p_name, int p_count) {
        const uint8_t idx = std::to_underlying(p_type);
        m_piece_pools[idx].reserve(p_count);
        for (int i = 1; i <= p_count; ++i) {
            std::string name = std::format("{}_{}", p_name, i);
            Entity id = p_query.FindFirstEntity(name);
            m_piece_pools[idx].push_back(id);
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

void ChessPresenter::Present() {
    cave::SceneQuery& query = m_host.SceneQuery();

    auto& writer = m_host.SceneWriter();

    for (uint8_t i = 0; i < 64; ++i) {
        const Square sq(i);
        bool visible = m_highlights.Test(sq);
        if (sq == m_focused) {
            visible = false;
        }
        const Entity tile = m_tiles[i];
        writer.SetProperty(tile,
                           cave::MeshRendererComponent_Id,
                           kVisibility,
                           visible);
    }

    Vector3f position = SquareToVec(m_focused);
    writer.SetProperty(m_selector,
                       cave::TransformComponent_Id,
                       kTranslationId,
                       position);
}

void ChessPresenter::InitBoard(const core::Position& p_position) {
    auto& writer = m_host.SceneWriter();

    // update pieces
    for (uint8_t p = 0; p < core::kPieceMax; ++p) {
        const Piece piece = static_cast<Piece>(p);
        const Bitboard bb = p_position.Bitboard(piece);
        auto& pool = m_piece_pools[p];

        int idx = 0;
        for (Square sq : bb.Squares()) {
            // @TODO: properly handle not enough entities in pool caused by promotion
            if (idx >= pool.size()) {
                break;
            }

            Entity e = pool[idx++];
            m_board[sq.Index()] = e;
        }

        // set the reset of the pieces invisible
        for (; idx < pool.size(); ++idx) {
            Entity e = pool[idx];
            writer.SetProperty(e, MeshRendererComponent_Id, kVisibility, false);
            writer.SetProperty(e, MeshRendererComponent_Id, kCastShadow, false);
        }
    }
}

// void ChessPresenter::SetEntityAt(SceneCommandWriter& p_writer,
//                                  core::Square p_sq,
//                                  Entity p_ent) {
//     m_board[p_sq.Index()] = p_ent;
//
//     Vector3f translation = SquareToVec(p_sq);
//     p_writer.SetProperty(p_ent, TransformComponent_Id, kTranslationId, translation);
//     p_writer.SetProperty(p_ent, MeshRendererComponent_Id, kVisibility, true);
//     p_writer.SetProperty(p_ent, MeshRendererComponent_Id, kCastShadow, true);
// }

void ChessPresenter::ClearSquare(SceneCommandWriter& p_writer,
                                 core::Square p_sq) {
    const Entity e = m_board[p_sq.Index()];
    m_board[p_sq.Index()] = Entity::Null();

    // Just move the piece to far away from the screen
    p_writer.SetProperty(e, MeshRendererComponent_Id, kVisibility, false);
    p_writer.SetProperty(e, MeshRendererComponent_Id, kCastShadow, false);
}

void ChessPresenter::MovePiece(Entity p_ent, core::Square p_from, core::Square p_to) {
    m_board[p_from.Index()] = Entity::Null();
    m_board[p_to.Index()] = p_ent;

    constexpr auto cid = TransformAnimationComponent_Id;
    auto& writer = m_host.SceneWriter();
    writer.AddComponent(p_ent, cid);
    writer.SetProperty(p_ent, cid, "begin"_sid, SquareToVec(p_from));
    writer.SetProperty(p_ent, cid, "end"_sid, SquareToVec(p_to));
    writer.SetProperty(p_ent, cid, "duration"_sid, 0.25f);
    writer.SetProperty(p_ent, cid, "playing"_sid, true);
    writer.SetProperty(p_ent, cid, "destroy_on_finish"_sid, true);
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

void ChessPresenter::ApplyMove(core::Move p_mv) {
    const Square from = p_mv.From();
    const Square to = p_mv.To();
    Entity src_piece = GetEntityAt(from);

    assert(src_piece.IsValid());

    auto& writer = m_host.SceneWriter();

    if (Entity captured_piece = GetEntityAt(to); captured_piece.IsValid()) {
        ClearSquare(writer, to);
    }

    MovePiece(src_piece, from, to);

    switch (p_mv.GetType()) {
        case MoveType::Normal:
            break;
        case MoveType::Castling: {
            CastleRookMove rook = GetCastleRookMove(from, to);
            MovePiece(m_board[rook.from.Index()], rook.from, rook.to);
        } break;
        case MoveType::Enpassant:
            m_host.Log().Warn(LogChannel::Game, "Handle enpassant");
            break;
        case MoveType::Promotion:
            m_host.Log().Warn(LogChannel::Game, "Handle promotion");
            break;
    }
}

}  // namespace chess
