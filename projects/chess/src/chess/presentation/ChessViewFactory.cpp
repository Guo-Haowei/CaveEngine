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

ChessViewFactory::ChessViewFactory(SceneCommandWriter& writer)
    : m_writer(writer) {
    m_materials[0] = "@res://materials/white.mat";
    m_materials[1] = "@res://materials/black.mat";
}

ecs::Entity ChessViewFactory::createTile(Square square, const TileCreatInfo& info) {
    constexpr Vec3f scale(1.0f, 0.05f, 1.0f);

    const auto [file, rank] = square.fileRank();

    Vec3f offset((float)rank, 0.05f, (float)file);

    Entity ent = m_writer.cubeObject(info.name ? info.name : square.uci(), { nullptr, info.color });
    m_writer.setProperty(ent, TransformComponent_Id, kScaleId, scale);
    m_writer.setProperty(ent, TransformComponent_Id, kTranslationId, offset);

    m_writer.setProperty(ent, MeshRendererComponent_Id, kVisibility, info.visible);
    m_writer.setProperty(ent, MeshRendererComponent_Id, kCastShadow, false);
    m_writer.setProperty(ent, MeshRendererComponent_Id, kTransparency, true);

    m_writer.attachChild(ent, info.parent);
    return ent;
}

ecs::Entity ChessViewFactory::createPiece(Square square, const PieceCreatInfo& info) {
    const PieceType piece_type = GetType(info.piece);
    const Color piece_color = GetColor(info.piece);
    DEV_ASSERT(piece_type != PieceType::Null);
    DEV_ASSERT(piece_color != Color::Null);

    const char* piece_name = core::GetPieceTypeName(piece_type);
    const char* color = (piece_color == Color::White ? "white" : "black");

    const int id = ++m_piece_counters[std::to_underlying(info.piece)];

    auto name = std::format("{}_{}_{}", color, piece_name, id);

    Entity ent = m_writer.meshObject(
        std::format("@res://models/{}.mesh", piece_name),
        name,
        m_materials[std::to_underlying(piece_color)]);

    const auto [file, rank] = square.fileRank();
    Vec3f translation(rank, 0, file);
    constexpr Vec3f scale = Vec3f(9);

    m_writer.setProperty(ent, TransformComponent_Id, kScaleId, scale);
    m_writer.setProperty(ent, TransformComponent_Id, kTranslationId, translation);
    if (piece_color == Color::Black) {
        m_writer.setProperty(ent, TransformComponent_Id, kRotationId, Vec4f::UnitY);
    }
    m_writer.setProperty(ent, MeshRendererComponent_Id, kVisibility, info.visible);
    m_writer.setProperty(ent, MeshRendererComponent_Id, kCastShadow, info.visible);

    m_writer.attachChild(ent, info.parent);
    return ent;
}

}  // namespace chess
