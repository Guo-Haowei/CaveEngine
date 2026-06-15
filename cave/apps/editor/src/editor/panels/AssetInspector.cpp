#include "AssetInspector.h"
#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/runtime/assets/TileMapAsset.h"

#include "editor/document/TileMapDocument.h"
#include "editor/services/DocumentService.h"
#include "editor/services/IconCache.h"
#include "editor/services/Workspace.h"
#include "editor/widgets/DragDrop.h"
#include "editor/widgets/Image.h"

// @TODO: remove private #include
#include "engine/private/ui/inputs.h"
#include "engine/private/ui/layout.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/assets/TileSetAsset.h"

#include "editor/EditorState.h"

namespace cave {

using namespace ::cave::math;

AssetInspector::AssetInspector(EditorState& editor,
                               EditorServices& editor_services)
    : EditorWindow(editor)
    , editor_services_(editor_services) {
}
void AssetInspector::onAttach() {
    IconCache& icons = editor_services_.iconCache();
    checkerboard_ = icons.GetIconHandle(IconName::Checkerboard);
}

void AssetInspector::tileMapLayerOverview(TileMapAsset& p_tile_map) {
    if (ImGui::Button(ICON_FA_SQUARE_PLUS " Add Layer")) {
        // p_tile_map.AddLayer("untitled layer");
        LOG_WARN("TODO: Add layer");
    }
    ImGui::Separator();

    for (int layer_id = 0; layer_id < 1; ++layer_id) {
        TileMapAsset& layer = p_tile_map;
        const bool is_layer_selected = true;

        ImGui::PushID(layer_id);

        if (is_layer_selected) {
            auto& style = ImGui::GetStyle();
            auto& colors = style.Colors;
            ImGui::PushStyleColor(ImGuiCol_ChildBg, colors[ImGuiCol_FrameBgHovered]);
        }

        ImGui::BeginGroup();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 0));

        ImGui::BeginGroup();

        ImGui::Dummy(ImVec2(8, 8));

        // if (ui::TextBox("layer", layer.GetName().c_str())) {
        //     // @TODO: notify dirty
        // }

        ImGui::SameLine();

        const bool is_visible = layer.visible();
        const char* label = is_visible ? ICON_FA_EYE : ICON_FA_EYE_SLASH;
        if (ImGui::Button(label)) {
            layer.visible(!is_visible);
        }

        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_TRASH_CAN)) {
            LOG_WARN("TODO: DELETE");
        }

        {

            const ImageAsset* image = nullptr;
            if (auto image_handle = layer.tileSetHandle().Get(); image_handle) {
                image = image_handle->GetHandle().Get();
            }

            Vector2f region_size(128, 128);
            ui::CenteredImage(image, region_size, checkerboard_);

            if (ImGui::IsItemClicked()) {
                // tool->SetActiveLayer(layer_id);
            }

            // @TODO: make an asset drop region
            // accept same type of assets, show tooltips, etc
            if (auto _handle = DragDropTarget(AssetType::TileSet); _handle.is_some()) {
                layer.SetTileSetGuid(_handle.unwrap_unchecked().GetGuid());
            }
        }

        ImGui::Dummy(ImVec2(8, 8));

        ImGui::EndGroup();
        ImGui::Separator();

        ImGui::PopStyleVar(2);
        ImGui::PopID();
        ImGui::EndGroup();

        if (is_layer_selected) {
            ImGui::PopStyleColor();
        }
    }
}

void AssetInspector::drawTileMap(TileMapDocument& doc) {
    TileMapAsset* tile_map = doc.handle<TileMapAsset>().Get();
    DEV_ASSERT(tile_map);

    if (ImGui::BeginTabBar("##MyTabs1")) {
        if (ImGui::BeginTabItem("Layer")) {
            tileMapLayerOverview(*tile_map);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();

    TileSetAsset* tile_set = tile_map->tileSetHandle().Get();
    if (tile_set) {
        auto handle = tile_set->GetHandle();
        const int column = tile_set->GetCol();
        const int row = tile_set->GetRow();
        if (auto image = handle.Get(); image) {
            tile_map_ctx_.sprite_selector.SelectSprite(*image, &column, &row);
        }
    }
}

static void DrawPhysicsTab(TileSetAsset& tile_set, SpriteSelector& sprite_selector) {
    int index = -1;
    if (auto selected = sprite_selector.GetSelections(); !selected.empty()) {
        auto [x, y] = selected.front();
        index = tile_set.GetCol() * y + x;
    }

    ToolBarButtonDesc add_square_button_desc = { ICON_FA_SQUARE " Box", "Add box collider",
                                                 [&]() {
                                                     if (tile_set.AddBoxCollider(index)) {
                                                         LOG_OK("Box collider added for {}", index);
                                                     } else {
                                                         LOG_ERROR("Failed to add box collider for {}", index);
                                                     }
                                                 } };

    ToolBarButtonDesc add_polygon_button_desc = { ICON_FA_DRAW_POLYGON " Polygon", "Add polygon collider",
                                                  [&]() {
                                                      LOG_WARN("Not implemented");
                                                  } };

    ToolBarButtonDesc add_circle_button_desc = { ICON_FA_CIRCLE " Circle", "Add circle collider",
                                                 [&]() {
                                                     LOG_WARN("Not implemented");
                                                 } };

    std::vector<const ToolBarButtonDesc*> tool_bar = {
        &add_square_button_desc,
        &add_polygon_button_desc,
        &add_circle_button_desc,
    };

    DrawToolBar(tool_bar);
    ImGui::Separator();
}

void AssetInspector::drawTileSet(IDocument& doc) {
    TileSetAsset* tile_set = doc.handle<TileSetAsset>().Get();
    DEV_ASSERT(tile_set);
    if (!tile_set) {
        return;
    }

    auto& sprite_selector = tile_map_ctx_.sprite_selector;
    {
        int column = tile_set->GetCol();
        int row = tile_set->GetRow();
        if (sprite_selector.EditSprite(&column, &row)) {
            tile_set->SetCol(column);
            tile_set->SetRow(row);
        }
    }

    ImGui::Separator();

    if (ImGui::BeginTabBar("TileSetPhysics")) {
        if (ImGui::BeginTabItem("Physics Layer")) {
            DrawPhysicsTab(*tile_set, sprite_selector);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();
    if (tile_set) {
        auto handle = tile_set->GetHandle();
        const int column = tile_set->GetCol();
        const int row = tile_set->GetRow();
        if (auto image = handle.Get(); image) {
            sprite_selector.SelectSprite(*image, &column, &row);
        }
    }
}

void AssetInspector::drawUIImpl() {
    DocId doc_id = editor_services_.workspace().focusedDoc();
    IDocument* doc = editor_services_.document().resolve(doc_id);
    if (!doc) {
        return;
    }

    if (auto tilemap = dynamic_cast<TileMapDocument*>(doc)) {
        drawTileMap(*tilemap);
        return;
    }

    IAsset* asset = doc->rawHandle().Get();
    switch (asset->GetType()) {
        case AssetType::TileMap: {
            drawTileMap(*static_cast<TileMapDocument*>(doc));
        } break;
        case AssetType::TileSet: {
            drawTileSet(*doc);
        } break;
        default:
            break;
    }
}

}  // namespace cave
