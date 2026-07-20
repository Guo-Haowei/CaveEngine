#include "TileSetEditor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/document/IDocument.h"

// @TODO: remove
#include "engine/private/runtime/assets/ImageAsset.h"

namespace cave {

namespace {

void DrawPhysicsTab(TileSetAsset& tile_set, SpriteSelector& sprite_selector) {
    int index = -1;
    if (auto selected = sprite_selector.GetSelections(); !selected.empty()) {
        auto [x, y] = selected.front();
        index = tile_set.col() * y + x;
    }

    ToolbarButtonDesc add_square_button_desc = {
        "TileSetEditor.physics.box",
        ICON_FA_SQUARE " Box", "Add box collider",
        [&]() {
            // if (index >= 0 && tile_set.addBoxCollider(index)) {
            //     LOG_OK("Box collider added for {}", index);
            // } else {
            //     LOG_ERROR("Failed to add box collider for {}", index);
            // }
        }
    };

    ToolbarButtonDesc add_polygon_button_desc = {
        "TileSetEditor.physics.polygon",
        ICON_FA_DRAW_POLYGON " Polygon", "Add polygon collider",
        [&]() {
            LOG_WARN("Not implemented");
        }
    };

    ToolbarButtonDesc add_circle_button_desc = {
        "TileSetEditor.circle.polygon",
        ICON_FA_CIRCLE " Circle", "Add circle collider",
        [&]() {
            LOG_WARN("Not implemented");
        }
    };

    Vector<const ToolbarButtonDesc*> tool_bar = {
        &add_square_button_desc,
        &add_polygon_button_desc,
        &add_circle_button_desc,
    };

    DrawToolbar(tool_bar);
    ImGui::Separator();
}

}  // namespace

void TileSetEditor::drawAssetInspector(IDocument& doc) {
    TileSetAsset* tile_set = doc.handle<TileSetAsset>().get();
    DEV_ASSERT(tile_set);
    if (!tile_set) {
        return;
    }

    {
        int column = tile_set->col();
        int row = tile_set->row();
        if (sprite_selector_.EditSprite(&column, &row)) {
            tile_set->col(column);
            tile_set->row(row);
        }
    }

    ImGui::Separator();

    if (ImGui::BeginTabBar("TileSetPhysics")) {
        if (ImGui::BeginTabItem("Physics Layer")) {
            DrawPhysicsTab(*tile_set, sprite_selector_);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();
    if (tile_set) {
        auto handle = tile_set->handle();
        const int column = tile_set->col();
        const int row = tile_set->row();
        if (auto image = handle.get(); image) {
            sprite_selector_.SelectSprite(*image, &column, &row);
        }
    }
}

}  // namespace cave
