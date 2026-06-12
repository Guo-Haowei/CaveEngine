#include "ChessViewFactory.h"

#include "cave/core/error/ErrorMacros.h"
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

ChessViewFactory::ChessViewFactory(SceneCommandWriter& writer, Entity parent)
    : writer_(writer)
    , parent_(parent) {
    materials_[0] = "@res://materials/white.mat";
    materials_[1] = "@res://materials/black.mat";
}

ecs::Entity ChessViewFactory::createTile(Square square, const TileInitInfo& info) {
    constexpr Vector3f scale(1.0f, 0.05f, 1.0f);

    const auto [file, rank] = square.fileRank();

    Vector3f offset((float)rank, 0.05f, (float)file);

    Entity ent = writer_.CreateCubeObject(info.name ? info.name : square.uci(), { nullptr, info.color });
    writer_.SetProperty(ent, TransformComponent_Id, kScaleId, scale);
    writer_.SetProperty(ent, TransformComponent_Id, kTranslationId, offset);

    writer_.SetProperty(ent, MeshRendererComponent_Id, kVisibility, visible_);
    writer_.SetProperty(ent, MeshRendererComponent_Id, kCastShadow, false);
    writer_.SetProperty(ent, MeshRendererComponent_Id, kTransparency, true);

    writer_.AttachChild(ent, info.parent);
    return ent;
}

ecs::Entity ChessViewFactory::createPiece(Square square, Piece piece) {
    const PieceType piece_type = GetType(piece);
    const Color piece_color = GetColor(piece);
    DEV_ASSERT(piece_type != PieceType::Null);
    DEV_ASSERT(piece_color != Color::Null);

    const char* piece_name = core::GetPieceTypeName(piece_type);
    const char* color = (piece_color == Color::White ? "white" : "black");

    const int id = ++piece_counters_[std::to_underlying(piece)];

    auto name = std::format("{}_{}_{}", color, piece_name, id);

    Entity ent = writer_.CreateMeshObject(
        std::format("@res://models/{}.mesh", piece_name),
        name,
        materials_[std::to_underlying(piece_color)]);

    const auto [file, rank] = square.fileRank();
    Vector3f translation(rank, 0, file);
    constexpr Vector3f scale = Vector3f(9);

    writer_.SetProperty(ent, TransformComponent_Id, kScaleId, scale);
    writer_.SetProperty(ent, TransformComponent_Id, kTranslationId, translation);
    if (piece_color == Color::Black) {
        writer_.SetProperty(ent, TransformComponent_Id, kRotationId, Vector4f::UnitY);
    }
    writer_.SetProperty(ent, MeshRendererComponent_Id, kVisibility, visible_);
    writer_.SetProperty(ent, MeshRendererComponent_Id, kCastShadow, visible_);

    writer_.AttachChild(ent, parent_);
    return ent;
}

}  // namespace chess
