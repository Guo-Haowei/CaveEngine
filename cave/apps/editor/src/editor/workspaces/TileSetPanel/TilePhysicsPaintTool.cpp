#include "TilePhysicsPaintTool.h"

// @TODO: refactor
#include "engine/private/runtime/ui/Inputs.h"
#include "engine/private/core/reflection/MetaEditor.h"

namespace cave {

bool TilePhysicsPaintTool::drawPaintProperties(TileSetAsset*) {
    bool changed = false;

    static constexpr const char* kTools[] = {
        "Assign AABB",
        "Remove",
    };

    int tool = static_cast<int>(m_physics_paint.tool);

    ImGui::TextUnformatted("Painting");
    ImGui::SetNextItemWidth(-1.0f);

    if (ImGui::Combo("##PhysicsPaintTool", &tool, kTools, std::size(kTools))) {
        m_physics_paint.tool = static_cast<PhysicsPaintTool>(tool);
        changed = true;
    }

    if (m_physics_paint.tool == PhysicsPaintTool::Remove) {
        ImGui::TextDisabled("Paint over tiles to remove their colliders.");
        return changed;
    }

    ImGui::Spacing();

    changed |= DrawEnumDropDown<CollisionType>("Type", m_physics_paint.collision, ui::kDefaultColumnWidth);

    changed |= ui::DrawBitMask32("Mask", m_physics_paint.mask);

    ImGui::Spacing();
    ImGui::TextUnformatted("Collider Shape");

    changed |= drawPhysicsShapeEditor(m_physics_paint.shape);

    ImGui::Spacing();

    if (ImGui::Button("Full Tile")) {
        m_physics_paint.shape = Box2{ Vec2f::Zero, Vec2f::One };

        changed = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("Top Half")) {
        m_physics_paint.shape = Box2{ Vec2f{ 0.0f, 0.0f }, Vec2f{ 1.0f, 0.5f } };
        changed = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("Bottom Half")) {
        m_physics_paint.shape = Box2{ Vec2f{ 0.0f, 0.5f }, Vec2f{ 1.0f, 1.0f } };
        changed = true;
    }

    return changed;
}

bool TilePhysicsPaintTool::drawPhysicsShapeEditor(Box2& shape) {
    bool changed = false;

    const float size = math::min(ImGui::GetContentRegionAvail().x, 240.0f);

    const ImVec2 canvas_size{ math::max(size, 64.0f), math::max(size, 64.0f) };

    const ImVec2 min_ss = ImGui::GetCursorScreenPos();

    const ImVec2 max_ss{ min_ss.x + canvas_size.x, min_ss.y + canvas_size.y };

    ImGui::InvisibleButton("##PhysicsShapeEditor", canvas_size, ImGuiButtonFlags_MouseButtonLeft);

    const bool hovered = ImGui::IsItemHovered();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddRectFilled(
        min_ss,
        max_ss,
        IM_COL32(40, 47, 58, 255));

#if 0
    drawCheckerboard(
        *draw_list,
        min_ss,
        max_ss,
        12.0f);
#endif

    auto toScreen = [&](Vec2f uv) {
        return ImVec2{ min_ss.x + uv.x * canvas_size.x,
                       min_ss.y + uv.y * canvas_size.y };
    };

    auto toUv = [&](ImVec2 point_ss) {
        return Vec2f{ math::clamp((point_ss.x - min_ss.x) / canvas_size.x, 0.0f, 1.0f),
                      math::clamp((point_ss.y - min_ss.y) / canvas_size.y, 0.0f, 1.0f) };
    };

    Vec2f box_min = shape.min();
    Vec2f box_max = shape.max();

    ImVec2 box_min_ss = toScreen(box_min);
    ImVec2 box_max_ss = toScreen(box_max);

    draw_list->AddRectFilled(box_min_ss, box_max_ss, IM_COL32(60, 180, 205, 110));
    draw_list->AddRect(box_min_ss, box_max_ss, IM_COL32(120, 225, 245, 255), 0.0f, 0, 2.0f);

    constexpr float kHandleRadius = 6.0f;

    const ImVec2 handles[] = {
        box_min_ss,
        ImVec2{ box_max_ss.x, box_min_ss.y },
        box_max_ss,
        ImVec2{ box_min_ss.x, box_max_ss.y },
    };

    for (const ImVec2& handle : handles) {
        draw_list->AddCircleFilled(handle, kHandleRadius, IM_COL32(245, 245, 245, 255));
        draw_list->AddCircle(handle, kHandleRadius, IM_COL32(40, 40, 40, 255));
    }

    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        const ImVec2 mouse_ss = ImGui::GetMousePos();

        float best_distance_sq = kHandleRadius * kHandleRadius * 4.0f;

        m_physics_paint.drag_handle = -1;

        for (int i = 0; i < 4; ++i) {
            const float dx = mouse_ss.x - handles[i].x;
            const float dy = mouse_ss.y - handles[i].y;
            const float distance_sq = dx * dx + dy * dy;
            if (distance_sq < best_distance_sq) {
                best_distance_sq = distance_sq;
                m_physics_paint.drag_handle = i;
            }
        }

        m_physics_paint.dragging = m_physics_paint.drag_handle >= 0;
    }

    if (m_physics_paint.dragging && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        const Vec2f mouse_uv = toUv(ImGui::GetMousePos());

        switch (m_physics_paint.drag_handle) {
            case 0:
                box_min = mouse_uv;
                break;
            case 1:
                box_max.x = mouse_uv.x;
                box_min.y = mouse_uv.y;
                break;
            case 2:
                box_max = mouse_uv;
                break;
            case 3:
                box_min.x = mouse_uv.x;
                box_max.y = mouse_uv.y;
                break;
        }

        const Vec2f normalized_min = math::min(box_min, box_max);
        const Vec2f normalized_max = math::max(box_min, box_max);
        shape = Box2{ normalized_min, normalized_max };
        changed = true;
    }

    if (m_physics_paint.dragging && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        m_physics_paint.dragging = false;
        m_physics_paint.drag_handle = -1;
    }

    return changed;
}

bool TilePhysicsPaintTool::paintPhysics(TileSetAsset& tile_set, const AtlasHit& hit) {
    TileDefinition* definition = tile_set.findTileDefinition(hit.index);
    if (!definition) return false;

    if (m_physics_paint.tool == PhysicsPaintTool::Remove) {
        if (definition->collision == CollisionType::None) {
            return false;
        }
        definition->collision = CollisionType::None;
        definition->collision_shape = Box2{ Vec2f::Zero, Vec2f::One };

        return true;
    }

    if (definition->collision == m_physics_paint.collision &&
        definition->collision_shape == m_physics_paint.shape &&
        definition->mask == m_physics_paint.mask) {
        return false;
    }
    definition->collision = m_physics_paint.collision;
    definition->collision_shape = m_physics_paint.shape;
    definition->mask = m_physics_paint.mask;

    return true;
}

bool TilePhysicsPaintTool::handleAtlasPainting(TileSetAsset& tile_set,
                                               const AtlasWidgetResult& result,
                                               const ImageCanvasInput& input) {

    if (!result.pointer.hovered) {
        return false;
    }

    if (result.pointer.stroke_started && result.pointer.left_pressed) {
        return handleAtlasPainting(tile_set, result, input);
    }

    return false;
}

void TilePhysicsPaintTool::drawOverlay(const TileDefinition& definition,
                                       const AtlasLayout& layout,
                                       const AtlasWidgetResult& result) {
    if (definition.collision == CollisionType::None) {
        return;
    }

    const Box2 tile_rect = layout.cellRectPx(definition.id);
    const Vec2f tile_size = layout.cellSizePx();

    const Vec2f collider_min_px = tile_rect.min() + definition.collision_shape.min() * tile_size;
    const Vec2f collider_max_px = tile_rect.min() + definition.collision_shape.max() * tile_size;

    const ImVec2 collider_min_ss = result.imageToScreen(collider_min_px);

    const ImVec2 collider_max_ss = result.imageToScreen(collider_max_px);

    result.draw_list->AddRectFilled(collider_min_ss,
                                    collider_max_ss,
                                    IM_COL32(40, 190, 220, 100));

    result.draw_list->AddRect(collider_min_ss,
                              collider_max_ss,
                              IM_COL32(80, 225, 245, 255),
                              0.0f,
                              ImDrawFlags_None,
                              2.0f);
}

}  // namespace cave
