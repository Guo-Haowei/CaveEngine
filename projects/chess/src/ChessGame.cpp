#include "ChessGame.h"

#include "core/Piece.h"

#include "cave/core/diagnostics/ILogger.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/framework/IInputService.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace cave::chess {

using math::Vector3f;
using math::Vector4f;

static constexpr const char* kPieceNameTable[kPieceTypeMax]{
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
        const Color piece_color = GetColor(p_piece);
        DEV_ASSERT(piece_type != PieceType::Null);
        DEV_ASSERT(piece_color != Color::Null);

        const char* piece_name = kPieceNameTable[std::to_underlying(piece_type)];
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

        cb.SetProperty(piece, TransformComponent_Id, kScaleId, kScale);
        cb.SetProperty(piece, TransformComponent_Id, kTranslationId, translation);
        if (piece_color == Color::Black) {
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

}  // namespace cave::chess

namespace cave {

void ChessGame::OnModuleLoaded(IHostServices& p_host) {

    p_host.Log().Print(LogLevel::LOG_LEVEL_OK, "ChessGame Loaded\n");

    // @TODO: split to spawn pieces and place pieces
    SpawnPieces(p_host);
}

void ChessGame::Tick(IHostServices& p_host, const FrameTime& p_time) {
    unused(p_time);

    IInputService& input = p_host.Input();
    if (input.IsActionJustPressed(StringId("ui_up"))) {
        p_host.Log().Print(LOG_LEVEL_NORMAL, "ui_up just pressed");
    }
    if (input.IsActionPressed(StringId("ui_down"))) {
        p_host.Log().Print(LOG_LEVEL_NORMAL, "ui_down pressed");
    }
}

void ChessGame::SpawnPieces(IHostServices& p_host) {
    using chess::Piece;
    using ecs::Entity;

    Entity offset_node = p_host.SceneQuery().FindEntityByName("transform");
    DEV_ASSERT(offset_node.IsValid());

    SceneCommandWriter& writer = p_host.SceneWriter();
    writer.SetNoSave(true);
    chess::ChessSpawner spawner(writer, offset_node);

    std::array<int, chess::kPieceMax> counter{ 0 };

    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            const Piece p = chess::kInitialBoard[rank][file];
            if (p == Piece::Null) continue;
            spawner.SpawnPiece(p, file, rank, ++counter[std::to_underlying(p)]);
        }
    }
}

}  // namespace cave
