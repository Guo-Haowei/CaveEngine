#include "TileSetPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "editor/services/DocumentService.h"
#include "editor/services/EditorServices.h"
#include "editor/services/IconCache.h"
#include "editor/services/SceneEditService.h"
#include "editor/widgets/Image.h"

// @TODO: refactor
#include "engine/private/runtime/ui/Inputs.h"
#include "engine/private/core/reflection/MetaEditor.h"

namespace cave {

using namespace ::cave::math;

namespace {

// @TODO: refactor this function
bool SetupTileSet(TileSetAsset& tile_set) {
    bool dirty = false;
    if (ImGui::BeginTabBar("TileSetModes")) {
        int colomn = tile_set.col();
        int row = tile_set.row();
        if (ImGui::BeginTabItem("Setup")) {
            if (ImGui::InputInt("column", &colomn)) {
                tile_set.setCol(std::max(colomn, 1));
                dirty = true;
            }
            if (ImGui::InputInt("row", &row)) {
                tile_set.setRow(std::max(row, 1));
                dirty = true;
            }
            if (ImGui::Button("generate tiles")) {
                tile_set.generateTiles();
                dirty = true;
            }

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    return dirty;
}

bool DrawTileDefinition(TileDefinition& definition) {
    bool changed = false;

    ImGui::PushID(static_cast<int>(definition.id));

    ImGui::Text("Tile %u", definition.id);
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Physics", 0)) {
        ImGui::Indent();

        DrawEnumDropDown<CollisionType>("Type", definition.collision, ui::kDefaultColumnWidth);

        if (definition.collision != CollisionType::None) {
            ImGui::Spacing();

            Vec2f min = definition.collision_shape.min();
            Vec2f max = definition.collision_shape.max();

            if (ui::Float2("Min", min)) {
                max = math::max(max, min);
                definition.collision_shape = Box2{ min, max };
                changed = true;
            }

            if (ui::Float2("Max", max, 1.0f)) {
                min = math::min(min, max);
                definition.collision_shape = Box2{ min, max };
                changed = true;
            }

            // Optional normalization to tile-local coordinates.
            Vec2f clamped_min = math::clamp(definition.collision_shape.min(), Vec2f::Zero, Vec2f::One);
            Vec2f clamped_max = math::clamp(definition.collision_shape.max(), Vec2f::Zero, Vec2f::One);

            if (clamped_min != definition.collision_shape.min() ||
                clamped_max != definition.collision_shape.max()) {
                definition.collision_shape = Box2{ clamped_min, clamped_max };
                changed = true;
            }

            ImGui::Spacing();

            if (ui::DrawBitMask32("Mask", definition.mask)) {
                changed = true;
            }
        }
        ImGui::Unindent();
    }

    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Animation", 0)) {
        ImGui::Indent();

        Option<int> pending_delete;

        for (int i = 0; i < static_cast<int>(definition.animation.size()); ++i) {
            TileFrame& frame = definition.animation[i];

            ImGui::PushID(i);

            ImGui::SeparatorText(std::format("Frame {}", i).c_str());

            int atlas_index = static_cast<int>(frame.atlas_index);

            if (ui::InputInt("Atlas Index", atlas_index)) {
                frame.atlas_index = static_cast<uint32_t>(std::max(atlas_index, 0));

                changed = true;
            }

            if (ui::DragFloat("Duration", frame.duration, 0.01f, 0.01f, 10.0f)) {
                changed = true;
            }

            if (ImGui::Button(ICON_FA_TRASH_CAN " Remove")) {
                pending_delete = Some(i);
            }

            ImGui::PopID();
        }

        if (pending_delete) {
            definition.animation.erase(definition.animation.begin() + pending_delete.unwrap_unchecked());
            changed = true;
        }

        if (ImGui::Button(ICON_FA_PLUS " Add Frame")) {
            TileFrame frame;

            if (!definition.animation.empty()) {
                frame.atlas_index = definition.animation.back().atlas_index;
                frame.duration = definition.animation.back().duration;
            } else {
                frame.atlas_index = definition.id;
            }

            definition.animation.push_back(frame);
            changed = true;
        }

        ImGui::Unindent();
    }

    ImGui::PopID();
    return changed;
}

}  // namespace

TileSetPanel::TileSetPanel(EngineServices& engine_services,
                           EditorServices& editor_services)
    : m_engine_services(engine_services)
    , m_editor_services(editor_services) {

    m_checkerboard = editor_services.iconCache().getIconHandle(IconName::Checkerboard);
}

bool TileSetPanel::drawTileSource(TileSetAsset* tile_set) {
    bool dirty = false;
    unused(tile_set);

    ImGui::TableSetColumnIndex(0);

    ImGui::BeginChild("##TileSourcesColumn", ImVec2{ 0.0f, 0.0f });

    if (ImGui::BeginTabBar("##TileSourceTabs")) {
        if (ImGui::BeginTabItem("Tile Sources")) {
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Patterns")) {
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::BeginChild("##TileSourceList",
                      ImVec2{ 0.0f, -ImGui::GetFrameHeightWithSpacing() },
                      ImGuiChildFlags_Borders);

    ImGui::EndChild();

    const float button_size = ImGui::GetFrameHeight();

    ImGui::Button(ICON_FA_TRASH_CAN, ImVec2{ button_size, 0.0f });

    ImGui::SameLine();

    ImGui::Button(ICON_FA_PLUS, ImVec2{ button_size, 0.0f });

    ImGui::SameLine();

    ImGui::Button(ICON_FA_ELLIPSIS_VERTICAL, ImVec2{ button_size, 0.0f });

    ImGui::EndChild();

    return dirty;
}

bool TileSetPanel::drawTileProperties(TileSetAsset* tile_set) {
    bool dirty = false;

    ImGui::TableSetColumnIndex(1);

    ImGui::BeginChild("##TileToolsColumn", ImVec2{ 0.0f, 0.0f });

    auto mode_button = [&](Property value, const char* label) {
        const bool active = m_mode == value;
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
        }

        if (ImGui::Button(label)) {
            m_mode = value;
        }

        if (active) {
            ImGui::PopStyleColor();
        }
    };

    mode_button(Property::Setup, ICON_FA_SCREWDRIVER_WRENCH " Setup");
    ImGui::SameLine();
    mode_button(Property::SelectedTile, ICON_FA_ARROW_POINTER " Select");
    ImGui::SameLine();
    mode_button(Property::Paint, ICON_FA_PAINTBRUSH " Paint");
    ImGui::Separator();

    switch (m_mode) {
        case Property::Setup: {
            ImGui::TextUnformatted("Setup Properties");
            if (tile_set) {
                if (SetupTileSet(*tile_set)) {
                    dirty = true;
                }
            }
        } break;
        case Property::SelectedTile: {
            ImGui::TextUnformatted("Selected Tile Properties");
            if (tile_set) {
                for (TileDefinition& definition : tile_set->getTileDefinitionsMut()) {
                    DrawTileDefinition(definition);
                }
            }
        } break;
        case Property::Paint: {
            ImGui::TextUnformatted("Paint Properties:");
            ImGui::Spacing();

            drawPaintPropertySelector();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            switch (m_paint_property) {
                case PaintProperty::Physics: {
                    dirty |= drawPhysicsPaintProperties(tile_set);
                } break;
                case PaintProperty::Terrain: {
                    dirty |= drawTerrainPaintProperties(tile_set);
                } break;
                case PaintProperty::Animation: {
                    ImGui::TextDisabled("Animation painting is not implemented.");
                } break;
            }
        } break;
    }

    ImGui::EndChild();

    return dirty;
}

bool TileSetPanel::drawTerrainPaintProperties(TileSetAsset*) {
    bool changed = false;

    ImGui::TextUnformatted("Painting");

    ImGui::SetNextItemWidth(-1.0f);

    int terrain_id = static_cast<int>(m_terrain_paint.terrain_id.value);
    if (ImGui::InputInt("##TerrainId", &terrain_id)) {
        m_terrain_paint.terrain_id.value =
            static_cast<TerrainId::Type>(math::max(terrain_id, 1));
        changed = true;
    }

    ImGui::Spacing();
    ImGui::TextDisabled(
        "Left drag: paint terrain mask\n"
        "Right drag: erase terrain mask");

    return changed;
}

bool TileSetPanel::paintTerrainMask(TileSetAsset& tile_set,
                                    const AtlasHit& hit,
                                    bool erase) {
    TileDefinition* definition = tile_set.findTileDefinition(hit.index);
    if (!definition) return false;

    const uint32_t bit_index = hit.mask3x3Bit();
    const uint16_t bit = static_cast<uint16_t>(1u << bit_index);

    if (erase) {
        if ((definition->terrain_mask & bit) == 0) {
            return false;
        }

        definition->terrain_mask &= static_cast<uint16_t>(~bit);

        if (definition->terrain_mask == 0) {
            definition->terrain_id = TerrainId::null();
        }

        return true;
    }

    const bool terrain_changed = definition->terrain_id != m_terrain_paint.terrain_id;
    const bool bit_changed = (definition->terrain_mask & bit) == 0;

    if (!terrain_changed && !bit_changed) {
        return false;
    }

    definition->terrain_id = m_terrain_paint.terrain_id;
    definition->terrain_mask |= bit;
    return true;
}

bool TileSetPanel::drawPhysicsPaintProperties(TileSetAsset*) {

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

    // changed |= drawPhysicsShapeEditor(m_physics_paint.shape);

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

bool TileSetPanel::drawPhysicsShapeEditor(Box2& shape) {
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

bool TileSetPanel::paintPhysics(TileSetAsset& tile_set, const AtlasHit& hit) {
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

bool TileSetPanel::handleAtlasPainting(TileSetAsset& tile_set,
                                       const AtlasWidgetResult& result,
                                       const ImageCanvasInput& input) {

    bool dirty = false;

    if (!result.pointer.hovered) {
        m_last_painted_mask_cell = None();
        return false;
    }

    const AtlasHit& hit = result.pointer.hovered.unwrap_unchecked();

    if (m_paint_property == PaintProperty::Terrain) {

        const uint32_t key =
            hit.index * 9u +
            hit.mask3x3Bit();

        if (!input.left_down && !input.right_down) {
            m_last_painted_mask_cell = None();
            return false;
        }

        // Avoid repeatedly modifying the same subcell every frame.
        if (m_last_painted_mask_cell == Some(key)) {
            return false;
        }

        if (input.left_down) {
            dirty |= paintTerrainMask(tile_set, hit, false);
        } else if (input.right_down) {
            dirty |= paintTerrainMask(tile_set, hit, true);
        }

        m_last_painted_mask_cell = Some(key);

        if (dirty) {
            tile_set.refreshTerrainCache();
        }
        return dirty;
    }

    m_last_painted_mask_cell = None();

    if (m_paint_property == PaintProperty::Physics &&
        result.pointer.stroke_started &&
        result.pointer.left_pressed) {
        dirty |= paintPhysics(tile_set, result.pointer.left_pressed.unwrap_unchecked());
    }

    return dirty;
}

void TileSetPanel::drawHoveredTerrainCell(const AtlasLayout& layout,
                                          const AtlasHit& hit,
                                          const AtlasWidgetResult& result) {
    if (!result.draw_list) {
        return;
    }

    const Box2 tile_rect = layout.cellRectPx(hit.index);
    const Vec2f tile_size = layout.cellSizePx();
    const Vec2f subcell_size = tile_size / 3.0f;

    const Vec2i subcell = hit.mask3x3Cell();

    const Vec2f min_px{
        tile_rect.min().x + static_cast<float>(subcell.x) * subcell_size.x,
        tile_rect.min().y + static_cast<float>(subcell.y) * subcell_size.y,
    };

    const Vec2f max_px = min_px + subcell_size;

    result.draw_list->AddRectFilled(result.imageToScreen(min_px),
                                    result.imageToScreen(max_px),
                                    IM_COL32(255, 235, 90, 65));

    result.draw_list->AddRect(result.imageToScreen(min_px),
                              result.imageToScreen(max_px),
                              IM_COL32(255, 235, 90, 255),
                              0.0f,
                              ImDrawFlags_None,
                              2.0f);
}

bool TileSetPanel::drawAtlas(TileSetAsset* tile_set, ImageAsset* image) {
    bool dirty = false;

    ImGui::TableSetColumnIndex(2);

    ImGui::BeginChild("##TileAtlasColumn", ImVec2{ 0.0f, 0.0f });

    ImageCanvasInput input;
    input.pointer_ss = ImGui::GetMousePos();
    input.pointer_valid = true;

    input.zoom_steps = ImGui::GetIO().MouseWheel;

    input.left_double_clicked = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

    input.left_pressed = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    input.left_down = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    input.left_released = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

    input.right_pressed = ImGui::IsMouseClicked(ImGuiMouseButton_Right);
    input.right_down = ImGui::IsMouseDown(ImGuiMouseButton_Right);
    input.right_released = ImGui::IsMouseReleased(ImGuiMouseButton_Right);

    AtlasWidgetDesc desc;
    desc.id = "##TileAtlas";
    desc.texture = 0;

    desc.layout.image_size_px = { 64.0f, 64.0f };
    desc.layout.grid_size = { 1, 1 };
    desc.widget_size = { 0.0f, 0.0f };

    desc.show_toolbar = true;
    desc.show_checkerboard = true;
    desc.input = &input;

    if (tile_set) {
        desc.layout.grid_size = Vec2i{ tile_set->col(), tile_set->row() };
    }

    if (image && image->gpu_texture) {
        desc.texture = image->gpu_texture->GetHandle();
        desc.layout.image_size_px = Vec2f{ image->width, image->height };
    }

    const AtlasWidgetResult atlas_result = m_atlas_widget.draw(desc);

    // Paint before drawing overlays so changes appear immediately.
    if (tile_set && m_mode == Property::Paint) {
        dirty |= handleAtlasPainting(*tile_set, atlas_result, input);
    } else {
        m_last_painted_tile = None();
    }

    drawAtlasMetadataOverlays(tile_set, desc.layout, atlas_result);

    if (m_mode == Property::Paint && m_paint_property == PaintProperty::Terrain &&
        atlas_result.pointer.hovered) {
        drawHoveredTerrainCell(desc.layout,
                               atlas_result.pointer.hovered.unwrap_unchecked(),
                               atlas_result);
    }

    // BeginPopup must be called every frame, not only on right click.
    if (tile_set) {
        dirty |= drawTileContextPopup(*tile_set);
    }

    ImGui::EndChild();

    return dirty;
}

void TileSetPanel::drawAtlasMetadataOverlays(const TileSetAsset* tile_set,
                                             const AtlasLayout& layout,
                                             const AtlasWidgetResult& result) {
    if (!tile_set || !result.draw_list || !layout.valid()) {
        return;
    }

    for (const TileDefinition& definition : tile_set->getTileDefinitions()) {
        if (!layout.contains(definition.id)) {
            continue;
        }

        switch (m_paint_property) {
            case PaintProperty::Physics:
                drawPhysicsOverlay(definition, layout, result);
                break;
            case PaintProperty::Terrain:
                drawTerrainOverlay(definition, layout, result);
                break;
            case PaintProperty::Animation:
                break;
        }
    }
}

void TileSetPanel::drawPhysicsOverlay(const TileDefinition& definition,
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

void TileSetPanel::drawTerrainOverlay(const TileDefinition& definition,
                                      const AtlasLayout& layout,
                                      const AtlasWidgetResult& result) {
    if (!definition.terrain_id.valid()) {
        return;
    }

    const Box2 tile_rect = layout.cellRectPx(definition.id);

    const ImVec2 tile_min_ss = result.imageToScreen(tile_rect.min());
    const ImVec2 tile_max_ss = result.imageToScreen(tile_rect.max());

    result.draw_list->AddRectFilled(tile_min_ss,
                                    tile_max_ss,
                                    IM_COL32(230, 195, 60, 35));

    drawTerrainMaskOverlay(definition,
                           tile_rect,
                           layout,
                           result);
}

void TileSetPanel::drawTerrainMaskOverlay(const TileDefinition& definition,
                                          const Box2& tile_rect,
                                          const AtlasLayout& layout,
                                          const AtlasWidgetResult& result) {
    const Vec2f tile_size = layout.cellSizePx();
    const Vec2f subcell_size = tile_size / 3.0f;

    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            const int bit_index = y * 3 + x;
            const uint16_t bit = static_cast<uint16_t>(1u << bit_index);

            if ((definition.terrain_mask & bit) == 0) {
                continue;
            }

            const Vec2f subcell_min_px = tile_rect.min() + Vec2f(x, y) * subcell_size;
            const Vec2f subcell_max_px = subcell_min_px + subcell_size;

            constexpr ImU32 fill_color = IM_COL32(240, 60, 60, 150);

            result.draw_list->AddRectFilled(result.imageToScreen(subcell_min_px),
                                            result.imageToScreen(subcell_max_px),
                                            fill_color);

            result.draw_list->AddRect(result.imageToScreen(subcell_min_px),
                                      result.imageToScreen(subcell_max_px),
                                      IM_COL32(245, 100, 100, 180),
                                      0.0f,
                                      ImDrawFlags_None,
                                      1.0f);
        }
    }
}

bool TileSetPanel::drawTileContextPopup(TileSetAsset& tile_set) {
    bool dirty = false;

    if (!ImGui::BeginPopup("##TileContextPopup")) {
        return false;
    }

    if (!m_context_tile) {
        ImGui::TextDisabled("No tile selected");

        ImGui::EndPopup();
        return false;
    }

    const uint32_t tile_index = m_context_tile.unwrap_unchecked();

    ImGui::Text("Tile %u", tile_index);

    ImGui::Separator();

    TileDefinition& definition = tile_set.getOrCreateTile(tile_index);

    if (ImGui::MenuItem("Clear Physics", nullptr, false, definition.collision != CollisionType::None)) {
        definition.collision = CollisionType::None;
        definition.collision_shape = Box2{ Vec2f::Zero,
                                           Vec2f::One };
        dirty = true;
    }

    if (ImGui::MenuItem("Clear Terrain", nullptr, false, definition.terrain_id.valid())) {
        definition.terrain_id = TerrainId::null();
        definition.terrain_mask = 0;
        dirty = true;
    }

    if (ImGui::MenuItem("Clear Animation", nullptr, false, !definition.animation.empty())) {
        definition.animation.clear();
        dirty = true;
    }

    ImGui::Separator();

    if (ImGui::MenuItem(
            "Clear All Properties")) {
        definition.collision =
            CollisionType::None;

        definition.collision_shape = Box2{ Vec2f::Zero, Vec2f::One };

        definition.terrain_id = TerrainId::null();
        definition.terrain_mask = 0;
        definition.animation.clear();

        dirty = true;
    }

    ImGui::EndPopup();

    return dirty;
}

void TileSetPanel::drawPaintPropertySelector() {
    static constexpr const char* kNames[] = { "Physics", "Terrain", "Animation" };
    ImGui::SetNextItemWidth(-1.0f);

    int property = static_cast<int>(m_paint_property);
    if (ImGui::Combo("Select a property editor", &property, kNames, std::size(kNames))) {
        m_paint_property = static_cast<PaintProperty>(property);

        m_last_painted_tile = None();
        m_atlas_widget.cancelStroke();
    }
}

void TileSetPanel::draw(SceneEditContext* context) {
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_Resizable |
                                      ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_SizingStretchProp;

    if (!ImGui::BeginTable("##TileSetEditorLayout", 3,
                           flags,
                           ImGui::GetContentRegionAvail())) {
        return;
    }

    ImageAsset* image = nullptr;
    TileSetAsset* tile_set = nullptr;
    const bool valid_tile_set = context && context->tile.layer_entity.valid();
    if (valid_tile_set) {
        tile_set = context->tile.tile_set.get();
        image = context->tile.image.get();
    }

    ImGui::TableSetupColumn("##Sources", ImGuiTableColumnFlags_WidthFixed, 280.0f);
    ImGui::TableSetupColumn("##Tools", ImGuiTableColumnFlags_WidthFixed, 360.0f);
    ImGui::TableSetupColumn("##Atlas", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableNextRow();

    int dirty = 0;
    dirty |= (int)drawTileSource(tile_set);
    dirty |= (int)drawTileProperties(tile_set);
    dirty |= (int)drawAtlas(tile_set, image);

    ImGui::EndTable();

    if (valid_tile_set && dirty) {
        OpenDocDesc desc;
        desc.asset_type = AssetType::TileSet;
        desc.guid = context->tile.tile_set_guid;
        desc.focused = false;
        auto& document = m_editor_services.document();
        DocId doc_id = document.loadDoc(desc);
        document.markDirty(doc_id);
    }
}

}  // namespace cave
