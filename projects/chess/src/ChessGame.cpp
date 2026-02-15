#include "ChessGame.h"

#include "Piece.h"

#include "cave/core/diagnostics/ILogger.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

namespace cave {

using math::Vector3f;
using math::Vector4f;

void ChessGame::RegisterTypes(IHostServices& p_host) {
    unused(p_host);
}

void ChessGame::RegisterSystems(IHostServices& p_host) {
    unused(p_host);
}

void ChessGame::OnSceneBegin(Scene& p_scene,
                             IHostServices& p_host,
                             const GameInitDesc& p_init,
                             SceneCommandWriter& p_cb) {
    unused(p_init);

    p_host.Log().Print(LogLevel::LOG_LEVEL_OK, "hello from ChessGame\n");

    CreatePieces(p_scene, p_host, p_cb);
}

void ChessGame::OnSceneEnd(Scene& p_scene, IHostServices& p_host) {
    unused(p_scene);
    unused(p_host);
}

void ChessGame::Tick(Scene& p_scene, IHostServices& p_host, const FrameTime& p_time) {
    unused(p_scene);
    unused(p_host);
    unused(p_time);
}

static constexpr const char* kPieceNameTable[std::to_underlying(PieceType::_Count)]{
    "pawn",
    "knight",
    "bishop",
    "rook",
    "queen",
    "king",
};

struct ChessSpawner {
    static constexpr StringId kScaleId = StringId("scale");
    static constexpr StringId kTranslationId = StringId("translation");
    static constexpr StringId kRotationId = StringId("rotation");
    static constexpr Vector3f kScale = Vector3f(9);

    SceneCommandWriter& cb;
    ecs::Entity parent;
    const char* materials[2];

    ChessSpawner(SceneCommandWriter& p_cb, ecs::Entity p_parent)
        : cb(p_cb)
        , parent(p_parent) {
        materials[0] = "@res://materials/white.mat";
        materials[1] = "@res://materials/black.mat";
    }

    void SpawnPiece(Piece p_piece, int p_file, int p_rank, int p_id) {
        const PieceType piece_type = GetType(p_piece);
        const PieceColor piece_color = GetColor(p_piece);
        DEV_ASSERT(piece_type != PieceType::Null);
        DEV_ASSERT(piece_color != PieceColor::Null);

        const char* piece_name = kPieceNameTable[std::to_underlying(piece_type)];
        const char* color = (piece_color == PieceColor::White ? "white" : "black");

        auto name = std::format("{}_{}_{}",
                                color,
                                piece_name,
                                p_id);

        ecs::Entity piece = cb.CreateMeshObject(
            std::format("@res://models/{}.mesh", piece_name),
            name,
            materials[std::to_underlying(piece_color)]);

        Vector3f translation(p_rank, 0, p_file);

        cb.SetProperty(piece, TransformComponent_Id, kScaleId, kScale);
        cb.SetProperty(piece, TransformComponent_Id, kTranslationId, translation);
        if (piece_color == PieceColor::Black) {
            cb.SetProperty(piece, TransformComponent_Id, kRotationId, Vector4f(0, 1, 0, 0));
        }

        cb.AttachChild(piece, parent);
    }
};

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

void ChessGame::CreatePieces(Scene& p_scene, IHostServices& p_host, SceneCommandWriter& p_cb) {
    using ecs::Entity;

    Entity offset_node = SceneCommandWriter::FindEntityByName(p_scene, "transform");

    p_cb.SetNoSave(true);
    ChessSpawner spawner(p_cb, offset_node);

    std::array<int, std::to_underlying(Piece::Null)> counter{ 0 };

    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            const Piece p = kInitialBoard[rank][file];
            if (p == Piece::Null) continue;
            spawner.SpawnPiece(p, file, rank, ++counter[std::to_underlying(p)]);
        }
    }
}

}  // namespace cave
