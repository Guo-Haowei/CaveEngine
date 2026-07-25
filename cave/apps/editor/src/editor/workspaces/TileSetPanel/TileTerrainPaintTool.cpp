#include "TileTerrainPaintTool.h"

namespace cave {

bool TileTerrainPaintTool::paintMask(TileSetAsset& tile_set,
                                     const AtlasHit& hit,
                                     bool erase) {
    TileDefinition* definition = tile_set.findTileDefinition(hit.index);
    if (!definition) {
        return false;
    }

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

    const bool terrain_changed = definition->terrain_id != m_terrain_id;
    const bool bit_changed = (definition->terrain_mask & bit) == 0;

    if (!terrain_changed && !bit_changed) {
        return false;
    }

    definition->terrain_id = m_terrain_id;
    definition->terrain_mask |= bit;
    return true;
}

bool TileTerrainPaintTool::drawPaintProperties(TileSetAsset*) {
    bool changed = false;

    ImGui::TextUnformatted("Painting");

    ImGui::SetNextItemWidth(-1.0f);

    int terrain_id = static_cast<int>(m_terrain_id.value);
    if (ImGui::InputInt("##TerrainId", &terrain_id)) {
        m_terrain_id = TerrainId::from(terrain_id);
        changed = true;
    }

    ImGui::Spacing();
    ImGui::TextDisabled(
        "Left drag: paint terrain mask\n"
        "Right drag: erase terrain mask");

    return changed;
}

void TileTerrainPaintTool::drawOverlay(const TileDefinition& definition,
                                       const AtlasLayout& layout,
                                       const AtlasWidgetResult& result) {
    if (!definition.terrain_id.valid()) {
        return;
    }

    const Box2 tile_rect = layout.cellRectPx(definition.id);

    const ImVec2 min_ss = result.imageToScreen(tile_rect.min());
    const ImVec2 max_ss = result.imageToScreen(tile_rect.max());

    result.draw_list->AddRectFilled(min_ss, max_ss, IM_COL32(230, 195, 60, 35));

    drawMaskOverlay(definition, tile_rect, layout, result);
}

void TileTerrainPaintTool::drawMaskOverlay(const TileDefinition& definition,
                                           const Box2& tile_rect,
                                           const AtlasLayout& layout,
                                           const AtlasWidgetResult& result) {
    const Vec2f tile_size = layout.cellSizePx();
    const Vec2f subcell_size = tile_size / 3.0f;

    for (int bit_index = 0; bit_index < 9; ++bit_index) {
        const int y = bit_index / 3;
        const int x = bit_index % 3;
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

bool TileTerrainPaintTool::handleAtlasPainting(TileSetAsset& tile_set,
                                               const AtlasWidgetResult& result,
                                               const ImageCanvasInput& input) {
    if (!result.pointer.hovered) {
        m_last_painted_mask_cell = None();
        return false;
    }

    const AtlasHit& hit = result.pointer.hovered.unwrap_unchecked();

    bool dirty = false;

    const uint32_t key = hit.index * 9u + hit.mask3x3Bit();

    if (!input.left_down && !input.right_down) {
        m_last_painted_mask_cell = None();
        return false;
    }

    // Avoid repeatedly modifying the same subcell every frame.
    if (m_last_painted_mask_cell && m_last_painted_mask_cell.unwrap_unchecked() == key) {
        return false;
    }

    if (input.left_down) {
        dirty |= paintMask(tile_set, hit, false);
    } else if (input.right_down) {
        dirty |= paintMask(tile_set, hit, true);
    }

    m_last_painted_mask_cell = Some(key);

    // @TODO: do not refresh every time it changes
    if (dirty) {
        tile_set.refreshTerrainCache();
    }
    return dirty;
}

}  // namespace cave
