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

void ChessGame::OnSceneBegin(Scene&,
                             IHostServices& p_host,
                             const GameInitDesc& p_init,
                             SceneCommandBuffer& p_cb) {
    unused(p_init);

    p_host.Log().Print(LogLevel::LOG_LEVEL_OK, "hello from ChessGame\n");

    CreatePieces(p_host, p_cb);
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

void ChessGame::CreatePieces(IHostServices& p_host, SceneCommandBuffer& p_cb) {
    SceneExt scene_ext(p_host.AssetRegistry());

    if (&p_host != nullptr) {
        return;
    }

    // generate white pawns
    const float offset = -3.5f;
    const int rank = 1;
    PieceType piece_type = PieceType::Pawn;
    for (int file = 0; file < 8; ++file) {

        const char* piece_name = kPieceNameTable[std::to_underlying(piece_type)];
        auto name = std::format("white_{}_{}", piece_name, file + 1);
        ecs::Entity piece = scene_ext.CreateMeshObject(
            std::format("@res://models/{}.mesh", piece_name),
            p_cb,
            name,
            nullptr);

        Vector3f pos(rank, 0, file);
        Vector3f scale(9);
        pos.x += offset;
        pos.z += offset;

        p_cb.Add(piece, NoSaveTag_Id);

        p_cb.SetProperty(piece, TransformComponent_Id, StringId("scale"), scale);
        p_cb.SetProperty(piece, TransformComponent_Id, StringId("translation"), pos);

        p_cb.AttachRoot(piece);
    }
}

}  // namespace cave
