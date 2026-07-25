#include "TileMapPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/inspector/PropertyEditors.h"
#include "editor/services/DragDropService.h"
#include "editor/services/EditorServices.h"
#include "editor/services/IconCache.h"
#include "editor/services/SceneEditService.h"
#include "editor/widgets/Image.h"
#include "editor/widgets/SpriteSelector.h"

namespace cave {

using namespace ::cave::math;

TileMapPanel::TileMapPanel() {
    m_paint_mode = GridPaintMode::Brush;

    m_toolbar[0] = {
        "TilePaintTool.pencil",
        ICON_FA_PEN,
        "Pencil - paint individual tiles",
        [this]() { setPaintMode(GridPaintMode::Brush); },
        nullptr,
        [this]() { return m_paint_mode == GridPaintMode::Brush; },
    };
    m_toolbar[1] = {
        "TilePaintTool.line",
        ICON_FA_CHART_LINE,
        "Line - paint a straight line",
        [this]() { setPaintMode(GridPaintMode::Line); },
        nullptr,
        [this]() { return m_paint_mode == GridPaintMode::Line; },
    };
    m_toolbar[2] = {
        "TilePaintTool.rect",
        ICON_FA_SQUARE_PEN,
        "Rectangle - paint a filled rectangle",
        [this]() { setPaintMode(GridPaintMode::Rect); },
        nullptr,
        [this]() { return m_paint_mode == GridPaintMode::Rect; },
    };
    m_toolbar[3] = {
        "TilePaintTool.fill",
        ICON_FA_FILL,
        "Fill - replace a connected region",
        [this]() { setPaintMode(GridPaintMode::Fill); },
        nullptr,
        [this]() { return m_paint_mode == GridPaintMode::Fill; },
    };
}

void TileMapPanel::setPaintMode(GridPaintMode mode) {
    m_paint_mode = mode;
}

void TileMapPanel::draw(SceneEditContext* context) {
    DrawToolbar(m_toolbar);

    ImGui::Separator();

    TileSetAsset* tile_set = nullptr;
    ImageAsset* image = nullptr;
    if (context && context->tile.valid()) {
        context->tile.paint_mode = m_paint_mode;

        tile_set = context->tile.tile_set.get();
        if (tile_set) {
            image = tile_set->handle().get();
        }
    }

    if (ImGui::BeginTabBar("##TileSet")) {
        if (ImGui::BeginTabItem("Layer")) {
            if (tile_set) {
                const int column = tile_set->col();
                const int row = tile_set->row();
                if (image) {
                    m_sprite_selector.SelectSprite(*image, &column, &row);
                    auto selections = m_sprite_selector.GetSelections();
                    context->tile.selected_tile.clear();
                    context->tile.selected_tile.reserve(selections.size());
                    for (const auto& [x, y] : selections) {
                        const auto tile_id = TileId::from(y * column + x);
                        context->tile.selected_tile.push_back(tile_id);
                    }
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

}  // namespace cave
