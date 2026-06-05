#include "ChessPresenter.h"

#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "core/Position.h"

namespace chess {

using namespace cave;
using namespace cave::literals;

using cave::math::Vector3f;
using core::Bitboard;
using core::Piece;
using core::Square;

constexpr StringId kTranslationId = "translation"_sid;

static Vector3f SquareToPosition(const core::Square& p_sq) {
    auto [file, rank] = p_sq.FileRank();

    return Vector3f((float)rank, 0.0f, (float)file);
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

void ChessPresenter::Present(const PresentationContext& p_ctx) {
    cave::SceneQuery& query = p_ctx.host.SceneQuery();

    auto& writer = p_ctx.host.SceneWriter();

    for (uint8_t i = 0; i < 64; ++i) {
        const Square sq(i);
        bool visible = m_highlights.Test(sq);
        if (sq == m_focused) {
            visible = false;
        }
        const Entity tile = m_tiles[i];
        writer.SetProperty(tile,
                           cave::MeshRendererComponent_Id,
                           "visibility"_sid,
                           visible);
    }

    Vector3f position = SquareToPosition(m_focused);
    writer.SetProperty(m_selector,
                       cave::TransformComponent_Id,
                       kTranslationId,
                       position);
}

void ChessPresenter::RedrawPosition(cave::IHostServices& p_host, const core::Position& p_position) {
    // @TODO: refactor
    static constexpr StringId kTranslationId = "translation"_sid;
    static constexpr StringId kVisibility = "visibility"_sid;
    static constexpr StringId kCastShadow = "cast_shadow"_sid;

    auto& writer = p_host.SceneWriter();

    // update pieces
    for (uint8_t p = 0; p < core::kPieceMax; ++p) {
        const Piece piece = static_cast<Piece>(p);
        const Bitboard bb = p_position.Bitboard(piece);
        auto& pool = m_piece_pools[p];

        int idx = 0;
        for (Square sq : bb.Squares()) {
            const auto [file, rank] = sq.FileRank();
            Vector3f translation(rank, 0, file);
            // @TODO: properly handle not enough entities in pool caused by promotion
            if (idx >= pool.size()) break;
            Entity e = pool[idx++];
            writer.SetProperty(e, TransformComponent_Id, kTranslationId, translation);
            writer.SetProperty(e, MeshRendererComponent_Id, kVisibility, true);
            writer.SetProperty(e, MeshRendererComponent_Id, kCastShadow, true);
        }
        for (; idx < pool.size(); ++idx) {
            Entity e = pool[idx];
            writer.SetProperty(e, MeshRendererComponent_Id, kVisibility, false);
            writer.SetProperty(e, MeshRendererComponent_Id, kCastShadow, false);
        }
    }
}

}  // namespace chess
