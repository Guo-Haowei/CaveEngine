#include "ChessViewFactory.h"

#include "cave/core/ErrorMacros.h"
#include "cave/core/string/StringId.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

#include "chess/core/Square.h"

namespace chess {

using namespace ::cave;
using namespace ::cave::literals;
using namespace ::cave::math;
using namespace ::chess::core;

static constexpr StringId kScaleId = "scale"_sid;
static constexpr StringId kTranslationId = "translation"_sid;
static constexpr StringId kRotationId = "rotation"_sid;
static constexpr StringId kVisibility = "visibility"_sid;
static constexpr StringId kCastShadow = "cast_shadow"_sid;
static constexpr StringId kTransparency = "transparency"_sid;

ChessSpawner::ChessSpawner(SceneCommandWriter& writer, Entity parent)
    : writer_(writer)
    , piece_parent(parent) {
    materials[0] = "@res://materials/white.mat";
    materials[1] = "@res://materials/black.mat";
}

void ChessSpawner::SpawnTile(uint8_t file,
                             uint8_t rank,
                             const TileInitInfo& info) {
    constexpr Vector3f scale(1.0f, 0.05f, 1.0f);
    Vector3f offset((float)rank, 0.05f, (float)file);

    const Square sq = Square::FromFileRank(file, rank);
    ecs::Entity tile = writer_.CreateCubeObject(info.name ? info.name : sq.ToString(), { nullptr, info.color });
    writer_.SetProperty(tile, TransformComponent_Id, kScaleId, scale);
    writer_.SetProperty(tile, TransformComponent_Id, kTranslationId, offset);

    writer_.SetProperty(tile, MeshRendererComponent_Id, kVisibility, info.visible);
    writer_.SetProperty(tile, MeshRendererComponent_Id, kCastShadow, false);
    writer_.SetProperty(tile, MeshRendererComponent_Id, kTransparency, true);

    writer_.AttachChild(tile, info.parent);
}

void ChessSpawner::SpawnPiece(Piece p_piece, int p_file, int p_rank, int p_id) {
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

    ecs::Entity piece = writer_.CreateMeshObject(
        std::format("@res://models/{}.mesh", piece_name),
        name,
        materials[std::to_underlying(piece_color)]);

    Vector3f translation(p_rank, 0, p_file);
    constexpr Vector3f scale = Vector3f(9);

    writer_.SetProperty(piece, TransformComponent_Id, kScaleId, scale);
    writer_.SetProperty(piece, TransformComponent_Id, kTranslationId, translation);
    if (piece_color == Color::Black) {
        writer_.SetProperty(piece, TransformComponent_Id, kRotationId, Vector4f(0, 1, 0, 0));
    }

    writer_.AttachChild(piece, piece_parent);
}

}  // namespace chess
