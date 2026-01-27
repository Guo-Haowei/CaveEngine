#include "TileSetEditor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/private/assets/image_asset.h"
#include "engine/private/assets/tile_set_asset.h"
#include "engine/private/runtime/scene/CameraComponent.h"
#include "engine/private/ui/layout.h"

#include "editor/document/document.h"

namespace cave {

TileSetEditor::TileSetEditor(EditorState& p_editor, Viewer& p_viewer)
    : ViewerTab(p_editor, p_viewer, DIMENSION_2)
    , m_sprite_selector(SpriteSelector::SelectionMode::Single) {
}

TileSetEditor::~TileSetEditor() = default;

void TileSetEditor::OnCreateInternal(const Guid& p_guid) {
    m_document = std::make_unique<OldDocument>(p_guid);
}

void TileSetEditor::OnDestroy() {
}

void TileSetEditor::OnActivateInternal() {
}

void TileSetEditor::DrawMainView(const CameraComponent& p_camera) {
    unused(p_camera);
}

void TileSetEditor::DrawPhysicsTab(TileSetAsset& p_tile_set) {
    int index = -1;
    if (auto selected = m_sprite_selector.GetSelections(); !selected.empty()) {
        auto [x, y] = selected.front();
        index = p_tile_set.GetCol() * y + x;
    }

    ToolBarButtonDesc add_square_button_desc = { ICON_FA_SQUARE " Box", "Add box collider",
                                                 [&]() {
                                                     if (p_tile_set.AddBoxCollider(index)) {
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

void TileSetEditor::DrawAssetInspector() {
    TileSetAsset* tile_set = m_document->GetHandle<TileSetAsset>().Get();
    DEV_ASSERT(tile_set);

    std::vector<AssetChildPanel> descs = {
        {
            "SpriteTab",
            360,
            [&]() {
                if (!tile_set) {
                    return;
                }
                int column = tile_set->GetCol();
                int row = tile_set->GetRow();
                if (m_sprite_selector.EditSprite(&column, &row)) {
                    tile_set->SetCol(column);
                    tile_set->SetRow(row);
                }
            },
        },
        {
            "PhysicsTab",
            360,
            [&]() {
                if (ImGui::BeginTabBar("TileSetPhysics")) {
                    if (ImGui::BeginTabItem("Physics Layer")) {
                        DrawPhysicsTab(*tile_set);
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            },
        },
        {
            "PaintTab",
            0,
            [&]() {
                if (tile_set) {
                    auto handle = tile_set->GetHandle();
                    const int column = tile_set->GetCol();
                    const int row = tile_set->GetRow();
                    if (auto image = handle.Get(); image) {
                        m_sprite_selector.SelectSprite(*image, &column, &row);
                    }
                }
            },
        },
    };

    const float full_width = ImGui::GetContentRegionAvail().x;

    ui::DrawContents(full_width, descs);
}

OldDocument& TileSetEditor::GetDocument() const {
    return *m_document;
}

const std::vector<const ToolBarButtonDesc*> TileSetEditor::GetToolBarButtons() const {
    return {};
}

}  // namespace cave
