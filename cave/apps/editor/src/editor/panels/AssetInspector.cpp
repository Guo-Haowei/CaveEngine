#include "AssetInspector.h"
#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/diagnostics/Log.h"

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
#include "engine/private/runtime/assets/TileMapAsset.h"
#include "engine/private/runtime/assets/TileSetAsset.h"

#include "editor/EditorState.h"

namespace cave {

using namespace ::cave::math;

AssetInspector::AssetInspector(EditorState& editor,
                               EditorServices& editor_services)
    : EditorWindow(editor)
    , editor_services_(editor_services)
    , sprite_selector_(SpriteSelector::SelectionMode::Single) {
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

        const bool is_visible = layer.IsVisible();
        const char* label = is_visible ? ICON_FA_EYE : ICON_FA_EYE_SLASH;
        if (ImGui::Button(label)) {
            layer.SetVisible(!is_visible);
        }

        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_TRASH_CAN)) {
            LOG_WARN("TODO: DELETE");
        }

        {

            const ImageAsset* image = nullptr;
            if (auto image_handle = layer.GetTileSetHandle().Get(); image_handle) {
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

void AssetInspector::drawDocument(TileMapDocument& doc) {
    TileMapAsset* tile_map = doc.handle<TileMapAsset>().Get();

    TileSetAsset* tile_set = tile_map->GetTileSetHandle().Get();

    if (ImGui::BeginTabBar("##MyTabs1")) {
        if (ImGui::BeginTabItem("Layer")) {
            tileMapLayerOverview(*tile_map);
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
            sprite_selector_.SelectSprite(*image, &column, &row);
        }
    }
}

void AssetInspector::drawUIImpl() {
    DocId doc_id = editor_services_.workspace().focusedDoc();
    IDocument* doc = editor_services_.document().resolve(doc_id);

    if (auto tilemap = dynamic_cast<TileMapDocument*>(doc)) {
        drawDocument(*tilemap);
        return;
    }
}

}  // namespace cave
