#include "ChessGame.h"

#include "cave/core/diagnostics/ILogger.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

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

enum class PieceColor : uint8_t {
    White,
    Black,
    Null,
};

enum class PieceType : uint8_t {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,

    _Count,
    Null,
};

enum class Piece : uint8_t {
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

    Null,
};

static constexpr const char* kPieceNameTable[std::to_underlying(PieceType::_Count)]{
    "pawn",
    "knight",
    "bishop",
    "rook",
    "queen",
    "king",
};

constexpr PieceType GetType(Piece p_piece) {
    if (p_piece == Piece::Null) return PieceType::Null;
    constexpr uint8_t kPieceTypeCount = std::to_underlying(PieceType::_Count);
    const uint8_t type = std::to_underlying(p_piece) % kPieceTypeCount;
    return static_cast<PieceType>(type);
}

constexpr PieceColor GetColor(Piece p_piece) {
    if (p_piece == Piece::Null) return PieceColor::Null;
    constexpr uint8_t kPieceTypeCount = std::to_underlying(PieceType::_Count);
    const uint8_t type = (std::to_underlying(p_piece)) / kPieceTypeCount;
    return static_cast<PieceColor>(type);
}

static_assert(GetType(Piece::Null) == PieceType::Null);
static_assert(GetType(Piece::WPawn) == PieceType::Pawn);
static_assert(GetType(Piece::WKnight) == PieceType::Knight);
static_assert(GetType(Piece::WBishop) == PieceType::Bishop);
static_assert(GetType(Piece::WRook) == PieceType::Rook);
static_assert(GetType(Piece::WQueen) == PieceType::Queen);
static_assert(GetType(Piece::WKing) == PieceType::King);
static_assert(GetType(Piece::BPawn) == PieceType::Pawn);
static_assert(GetType(Piece::BKnight) == PieceType::Knight);
static_assert(GetType(Piece::BBishop) == PieceType::Bishop);
static_assert(GetType(Piece::BRook) == PieceType::Rook);
static_assert(GetType(Piece::BQueen) == PieceType::Queen);
static_assert(GetType(Piece::BKing) == PieceType::King);

static_assert(GetColor(Piece::Null) == PieceColor::Null);
static_assert(GetColor(Piece::WPawn) == PieceColor::White);
static_assert(GetColor(Piece::WKnight) == PieceColor::White);
static_assert(GetColor(Piece::WBishop) == PieceColor::White);
static_assert(GetColor(Piece::WRook) == PieceColor::White);
static_assert(GetColor(Piece::WQueen) == PieceColor::White);
static_assert(GetColor(Piece::WKing) == PieceColor::White);
static_assert(GetColor(Piece::BPawn) == PieceColor::Black);
static_assert(GetColor(Piece::BKnight) == PieceColor::Black);
static_assert(GetColor(Piece::BBishop) == PieceColor::Black);
static_assert(GetColor(Piece::BRook) == PieceColor::Black);
static_assert(GetColor(Piece::BQueen) == PieceColor::Black);
static_assert(GetColor(Piece::BKing) == PieceColor::Black);

struct ChessSpawner {
    static constexpr StringId kScaleId = StringId("scale");
    static constexpr StringId kTranslationId = StringId("translation");
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

        cb.Add(piece, NoSaveTag_Id);
        cb.SetProperty(piece, TransformComponent_Id, kScaleId, kScale);
        cb.SetProperty(piece, TransformComponent_Id, kTranslationId, translation);

        cb.AttachChild(piece, parent);
    }
};

void ChessGame::CreatePieces(Scene& p_scene, IHostServices& p_host, SceneCommandWriter& p_cb) {
    using ecs::Entity;

    Entity offset_node = SceneCommandWriter::FindEntityByName(p_scene, "transform");
    ChessSpawner spawner(p_cb, offset_node);

    for (int file = 0, id = 1; file < 8; ++file) {
        const int rank = 1;
        spawner.SpawnPiece(Piece::WPawn, file, rank, id++);
    }

    for (int file = 0, id = 1; file < 8; file += 7) {
        const int rank = 0;
        spawner.SpawnPiece(Piece::BRook, file, rank, id++);
    }
}

}  // namespace cave
