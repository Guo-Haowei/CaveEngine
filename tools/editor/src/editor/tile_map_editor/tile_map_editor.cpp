#include "tile_map_editor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/assets/image_asset.h"
#include "engine/assets/tile_set_asset.h"
#include "engine/input/input_event.h"
#include "engine/scene/entity_factory.h"
#include "editor/editor_layer.h"
#include "editor/editor_scene_manager.h"
#include "editor/widgets/widget.h"
#include "editor/viewer/viewer.h"
#include "editor/utility/imguizmo.h"
#include "editor/tile_map_editor/tile_map_document.h"

namespace cave {

TileMapEditor::TileMapEditor(EditorLayer& p_editor, Viewer& p_viewer)
    : ViewerTab(p_editor, p_viewer) {

    m_camera = std::make_unique<CameraComponent>();
    ViewerTab::CreateDefaultCamera2D(*m_camera.get());
}

void TileMapEditor::OnCreate(const Guid& p_guid) {
    ViewerTab::OnCreate(p_guid);

    m_document = std::make_shared<TileMapDocument>(p_guid, *this);

    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);

    m_tmp_scene = scene_manager->CreateTempScene(p_guid, [&]() {
        auto scene = std::make_shared<Scene>();
        auto root = EntityFactory::CreateTransformEntity(*scene, "tile_map_test_scene");
        scene->m_root = root;

        auto id = EntityFactory::CreateTileMapEntity(*scene, "tile_map");
        scene->AttachChild(id);

        TileMapRendererComponent* tile_map_renderer = scene->GetComponent<TileMapRendererComponent>(id);
        tile_map_renderer->SetResourceGuid(p_guid);
        return scene;
    });
}

void TileMapEditor::OnDestroy() {
    m_tmp_scene.reset();
}

void TileMapEditor::OnActivate() {
    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);
    scene_manager->OpenTempScene(m_tmp_scene);
}

void TileMapEditor::DrawMainView(const CameraComponent& p_camera) {
    ViewerTab::DrawMainView(p_camera);

    const Matrix4x4f proj_view = p_camera.GetProjectionViewMatrix();

    const Vector2f& canvas_min = m_viewer.GetCanvasMin();
    const Vector2f& canvas_size = m_viewer.GetCanvasSize();

    ImGuizmo::SetOrthographic(true);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(canvas_min.x, canvas_min.y, canvas_size.x, canvas_size.y);

    Matrix4x4f identity(1.0f);
    ImGuizmo::DrawGrid(proj_view, identity, 10.0f, ImGuizmo::GridPlane::XY);

    // @NOTE: shouldn't do it here,
    // move it do somewhere else
    m_document->FlushCommands();
}

void TileMapEditor::DrawAssetInspector() {
    TileMapAsset* tile_map = m_document->GetHandle<TileMapAsset>().Get();
    TileSetAsset* tile_set = tile_map->GetTileSetHandle().Get();

    std::vector<AssetChildPanel> descs = {
        {
            "LayerOverview",
            360,
            [&]() {
                if (ImGui::BeginTabBar("##MyTabs1")) {
                    if (ImGui::BeginTabItem("Layer")) {
                        TileMapLayerOverview(*tile_map);
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            },
        },
        {
            "SpriteTab",
            360,
            [&]() {
                int column = tile_set->GetCol();
                int row = tile_set->GetRow();
                if (m_sprite_selector.EditSprite(&column, &row)) {
                    tile_set->SetCol(column);
                    tile_set->SetRow(row);
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
        }
    };

    const float full_width = ImGui::GetContentRegionAvail().x;

    DrawContents(full_width, descs);
}

bool TileMapEditor::CursorToTile(const Vector2f& p_in, TileIndex& p_out) const {
    auto res = m_viewer.CursorToNDC(p_in);
    if (res.is_none()) {
        return false;
    }

    auto ndc_2 = res.unwrap_unchecked();
    Vector4f ndc{ ndc_2.x, ndc_2.y, 0.0f, 1.0f };

    const CameraComponent& cam = GetActiveCamera();
    const auto inv_proj_view = glm::inverse(cam.GetProjectionViewMatrix());

    Vector4f position = inv_proj_view * ndc;
    position /= position.w;

    p_out.x = static_cast<int16_t>(std::floor(position.x));
    p_out.y = static_cast<int16_t>(std::floor(position.y));

    return true;
}

Document& TileMapEditor::GetDocument() const {
    return *m_document.get();
}

bool TileMapEditor::HandleInput(const InputEvent* p_input_event) {
    if (auto e = dynamic_cast<const InputEventMouse*>(p_input_event); e) {
        if (!e->IsModiferPressed()) {
            if (e->IsButtonDown(MouseButton::LEFT)) {
                auto selections = m_sprite_selector.GetSelections();
                if (!selections.empty()) {
                    // @TODO: support multi tile editing
                    auto [x, y] = selections[0];
                    if (x >= 0 && y >= 0) {
                        TileMapAsset* tile_map = m_document->GetHandle<TileMapAsset>().Get();
                        TileSetAsset* tile_set = tile_map->GetTileSetHandle().Get();
                        uint32_t idx = y * tile_set->GetCol() + x;
                        m_document->RequestAdd(e->GetPos(), TileId(idx));
                    }
                }
                return true;
            }
            if (e->IsButtonDown(MouseButton::RIGHT)) {
                m_document->RequestErase(e->GetPos());
                return true;
            }
        }
    }

    return false;
}

const CameraComponent& TileMapEditor::GetActiveCameraInternal() const {
    DEV_ASSERT(m_camera);
    return *m_camera.get();
}

void TileMapEditor::TileMapLayerOverview(TileMapAsset& p_tile_map) {
    if (ImGui::Button(ICON_FA_SQUARE_PLUS " Add Layer")) {
        // p_tile_map.AddLayer("untitled layer");
    }
    ImGui::Separator();

    auto tool = dynamic_cast<TileMapEditor*>(m_editor.GetViewer().GetActiveTab());
    DEV_ASSERT(tool);

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

        if (DrawInputText("layer", layer.GetName())) {
            // @TODO: notify dirty
        }

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

        // next line

        {
            ImVec2 region_size(128, 128);

            const ImageAsset* image = nullptr;
            if (auto image_handle = layer.GetTileSetHandle().Get(); image_handle) {
                image = image_handle->GetHandle().Get();
            }

            auto checkerboard = m_editor.context.checkerboard;
            DEV_ASSERT(checkerboard && checkerboard->gpu_texture);

            CenteredImage(image, region_size, checkerboard->gpu_texture->GetHandle());

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

const std::vector<ToolBarButtonDesc>& TileMapEditor::GetToolBarButtons() const {
    static std::vector<ToolBarButtonDesc> s_buttons = {
        { ICON_FA_BRUSH, "TileMap editor mode",
          [&]() {
              LOG_WARN("TODO");
          } },
    };

    return s_buttons;
}

}  // namespace cave
