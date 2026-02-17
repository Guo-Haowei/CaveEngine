#include "ChessClient.h"

#include "core/Bitboard.h"
#include "core/Piece.h"

#include "cave/core/diagnostics/ILogger.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace chess {

using namespace cave;

using cave::ecs::Entity;
using cave::math::Vector2i;
using cave::math::Vector3f;
using cave::math::Vector4f;
using chess::core::Color;
using chess::core::Piece;
using chess::core::PieceType;
using chess::core::Square;

struct ChessSpawner {
    static constexpr StringId kScaleId = StringId("scale");
    static constexpr StringId kTranslationId = StringId("translation");
    static constexpr StringId kRotationId = StringId("rotation");
    static constexpr StringId kVisibility = StringId("visibility");
    static constexpr StringId kCastShadow = StringId("cast_shadow");
    static constexpr StringId kTransparency = StringId("transparency");

    SceneCommandWriter& cb;
    ecs::Entity piece_parent;
    ecs::Entity tile_parent;
    const char* materials[2];

    ChessSpawner(SceneCommandWriter& p_cb,
                 ecs::Entity p_tile_parent, ecs::Entity p_piece_parent)
        : cb(p_cb)
        , piece_parent(p_piece_parent)
        , tile_parent(p_tile_parent) {
        materials[0] = "@res://materials/white.mat";
        materials[1] = "@res://materials/black.mat";
    }

    void SpawnTiles(uint8_t p_file, uint8_t p_rank) {
        const Square sq = Square::FromFileRank(p_file, p_rank);
        const char* name = sq.ToString();

        constexpr Vector3f scale(1.0f, 0.05f, 1.0f);
        Vector3f offset((float)p_rank, 0.05f, (float)p_file);

        constexpr Vector4f color(0.0f, 1.f, 0.0980392173f, 0.501960814f);
        ecs::Entity tile = cb.CreateCubeObject(name, { nullptr, color });
        cb.SetProperty(tile, TransformComponent_Id, kScaleId, scale);
        cb.SetProperty(tile, TransformComponent_Id, kTranslationId, offset);

        cb.SetProperty(tile, MeshRendererComponent_Id, kVisibility, false);
        cb.SetProperty(tile, MeshRendererComponent_Id, kCastShadow, false);
        cb.SetProperty(tile, MeshRendererComponent_Id, kTransparency, true);

        cb.AttachChild(tile, tile_parent);
    }

    void SpawnPiece(Piece p_piece, int p_file, int p_rank, int p_id) {
        const PieceType piece_type = GetType(p_piece);
        const Color piece_color = GetColor(p_piece);
        DEV_ASSERT(piece_type != PieceType::Null);
        DEV_ASSERT(piece_color != Color::Null);

        const char* piece_name = core::GetPieceTypeName(piece_type);
        const char* color = (piece_color == Color::White ? "white" : "black");

        auto name = std::format("{}_{}_{}",
                                color,
                                piece_name,
                                p_id);

        ecs::Entity piece = cb.CreateMeshObject(
            std::format("@res://models/{}.mesh", piece_name),
            name,
            materials[std::to_underlying(piece_color)]);

        Vector3f translation(p_rank, 0, p_file);
        constexpr Vector3f scale = Vector3f(9);

        cb.SetProperty(piece, TransformComponent_Id, kScaleId, scale);
        cb.SetProperty(piece, TransformComponent_Id, kTranslationId, translation);
        if (piece_color == Color::Black) {
            cb.SetProperty(piece, TransformComponent_Id, kRotationId, Vector4f(0, 1, 0, 0));
        }

        cb.AttachChild(piece, piece_parent);
    }
};

// @TODO: use Position instead
static constexpr std::array<std::array<Piece, 8>, 8> kInitialBoard = { {
    { Piece::WR, Piece::WN, Piece::WB, Piece::WQ, Piece::WK, Piece::WB, Piece::WN, Piece::WR },
    { Piece::WP, Piece::WP, Piece::WP, Piece::WP, Piece::WP, Piece::WP, Piece::WP, Piece::WP },
    { Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null },
    { Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null },
    { Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null },
    { Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null },
    { Piece::BP, Piece::BP, Piece::BP, Piece::BP, Piece::BP, Piece::BP, Piece::BP, Piece::BP },
    { Piece::BR, Piece::BN, Piece::BB, Piece::BQ, Piece::BK, Piece::BB, Piece::BN, Piece::BR },
} };

void ChessClient::OnModuleLoaded(IHostServices& p_host) {
    p_host.Log().Print(LogLevel::LOG_LEVEL_OK, "ChessClient Loaded\n");

    // @TODO: move it to present layer
    SpawnObjects(p_host);
}

void ChessClient::OnModuleUnloaded(IHostServices& p_host) {
    unused(p_host);
}

void ChessClient::OnGameBegin(IHostServices& p_host) {
    m_chess_mode.OnGameBegin(p_host);
}

void ChessClient::OnGameEnd(IHostServices& p_host) {
    m_chess_mode.OnGameEnd(p_host);
}

void ChessClient::Tick(IHostServices& p_host, const FrameTime& p_time) {
    unused(p_time);

    m_chess_mode.Tick(p_host);
}

void ChessClient::SpawnObjects(IHostServices& p_host) {
    using chess::Piece;
    using ecs::Entity;

    Entity offset_node = p_host.SceneQuery().FindFirstEntity("transform");
    DEV_ASSERT(offset_node.IsValid());

    SceneCommandWriter& writer = p_host.SceneWriter();
    writer.SetNoSave(true);

    Entity piece_parent = writer.CreateTransformObject("pieces");
    Entity tile_parent = writer.CreateTransformObject("tiles");

    writer.AttachChild(piece_parent, offset_node);
    writer.AttachChild(tile_parent, offset_node);

    std::array<int, core::kPieceMax> counter{ 0 };

    chess::ChessSpawner spawner(writer, tile_parent, piece_parent);
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            spawner.SpawnTiles(file, rank);

            const Piece p = chess::kInitialBoard[rank][file];
            if (p == Piece::Null) continue;
            spawner.SpawnPiece(p, file, rank, ++counter[std::to_underlying(p)]);
        }
    }
}

}  // namespace chess
