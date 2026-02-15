#include "ChessGame.h"

#include "cave/core/diagnostics/ILogger.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneMutatorExt.h"

namespace cave {

using math::Vector3f;

void ChessGame::RegisterTypes(IHostServices& p_host) {
    unused(p_host);
}

void ChessGame::RegisterSystems(IHostServices& p_host) {
    unused(p_host);
}

void ChessGame::OnSceneBegin(Scene& p_scene,
                             IHostServices& p_host,
                             const GameInitDesc& p_init,
                             SceneCommandBuffer& p_cb) {
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

enum class PieceType : uint8_t {
    Null,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,

    _Count,
};

enum class Piece : uint8_t {
    Null,
    WPawn,
    WKnight,
    WBishop,
    WRook,
    WQueen,
    WKing,
    BPawn,
    BKnight,
    BBishop,
    BRook,
    BQueen,
    BKing,
};

static constexpr const char* kPieceNameTable[std::to_underlying(PieceType::_Count)]{
    "null",
    "pawn",
    "knight",
    "bishop",
    "rook",
    "queen",
    "king",
};

void ChessGame::CreatePieces(Scene& p_scene, IHostServices& p_host, SceneCommandBuffer& p_cb) {
    using ecs::Entity;

    SceneExt scene_ext(p_host.AssetRegistry());

    Entity offset_node = SceneExt::FindEntityByName(p_scene, "transform");

    auto create_piece = [&](PieceType piece_type, int file, int rank, int id) {
        const char* piece_name = kPieceNameTable[std::to_underlying(piece_type)];
        auto name = std::format("white_{}_{}", piece_name, id);
        Entity piece = scene_ext.CreateMeshObject(
            std::format("@res://models/{}.mesh", piece_name),
            p_cb,
            name,
            "@res://materials/white.mat");

        Vector3f pos(rank, 0, file);
        Vector3f scale(9);

        p_cb.Add(piece, NoSaveTag_Id);

        p_cb.SetProperty(piece, TransformComponent_Id, StringId("scale"), scale);
        p_cb.SetProperty(piece, TransformComponent_Id, StringId("translation"), pos);

        scene_ext.AttachChild(p_cb, piece, offset_node);
    };

    // generate white pawns
    PieceType piece_type = PieceType::Pawn;
    for (int file = 0, id = 1; file < 8; ++file) {
        const int rank = 1;
        create_piece(PieceType::Pawn, file, rank, id++);
    }

    for (int file = 0, id = 1; file < 8; file += 7) {
        const int rank = 0;
        create_piece(PieceType::Rook, file, rank, id++);
    }
}

}  // namespace cave
