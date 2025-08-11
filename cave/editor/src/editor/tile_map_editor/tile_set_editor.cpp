#include "tile_set_editor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/assets/image_asset.h"
#include "engine/assets/tile_set_asset.h"
#include "engine/scene/camera_component.h"

#include "editor/document/document.h"
#include "editor/widgets/widget.h"

namespace cave {

TileSetEditor::TileSetEditor(EditorLayer& p_editor, Viewer& p_viewer)
    : ViewerTab(p_editor, p_viewer)
    , m_sprite_selector(SpriteSelector::SelectionMode::Single) {

    m_camera = std::make_unique<CameraComponent>();
    ViewerTab::CreateDefaultCamera2D(*m_camera.get());
}

TileSetEditor::~TileSetEditor() = default;

bool TileSetEditor::HandleInput(const InputEvent* p_input_event) {
    unused(p_input_event);
    return false;
}

void TileSetEditor::OnCreate(const Guid& p_guid) {
    ViewerTab::OnCreate(p_guid);

    m_document = std::make_unique<Document>(p_guid);
}

void TileSetEditor::OnDestroy() {
}

void TileSetEditor::OnActivate() {
}

void TileSetEditor::DrawMainView(const CameraComponent& p_camera) {
    unused(p_camera);
}

void TileSetEditor::DrawPhysicsTab(TileSetAsset& p_tile_set) {
    unused(p_tile_set);

    int index = -1;
    if (auto selected = m_sprite_selector.GetSelections(); !selected.empty()) {
        auto [x, y] = selected.front();
        index = SpriteSelector::Pack(x, y);
    }

    ToolBarButtonDesc add_square_button_desc = { ICON_FA_SQUARE " Box", "Add box collider",
                                                 [&]() {
                                                     LOG_WARN("TODO");
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

    DrawContents(full_width, descs);
}

Document& TileSetEditor::GetDocument() const {
    return *m_document;
}

const CameraComponent& TileSetEditor::GetActiveCameraInternal() const {
    DEV_ASSERT(m_camera);
    return *m_camera.get();
}

const std::vector<const ToolBarButtonDesc*> TileSetEditor::GetToolBarButtons() const {
    return {};
}

}  // namespace cave
