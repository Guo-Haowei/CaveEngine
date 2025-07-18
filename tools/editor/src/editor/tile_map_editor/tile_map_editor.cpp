#include "tile_map_editor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/assets/assets.h"
#include "engine/input/input_event.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/entity_factory.h"
#include "editor/editor_layer.h"
#include "editor/editor_scene_manager.h"
#include "editor/widget.h"
#include "editor/viewer/viewer.h"
#include "editor/utility/imguizmo.h"
#include "editor/tile_map_editor/tile_map_document.h"

// @TODO: refactor
#include "engine/assets/assets.h"
#include "engine/assets/sprite_asset.h"

namespace cave {

TileMapEditor::TileMapEditor(EditorLayer& p_editor, Viewer& p_viewer)
    : ViewerTab(p_editor, p_viewer) {
    m_camera = ViewerTab::CreateDefaultCamera2D();

    m_asset_registry = m_editor.GetApplication()->GetAssetRegistry();
    m_checkerboard_handle = m_asset_registry->FindByPath<ImageAsset>("@res://images/checkerboard.png").unwrap();
}

void TileMapEditor::OnCreate(const Guid& p_guid) {
    ViewerTab::OnCreate(p_guid);

    m_document = std::make_shared<TileMapDocument>(p_guid, *this);

    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);

    m_tmp_scene = scene_manager->OpenTemporaryScene(p_guid, [&]() {
        auto scene = std::make_shared<Scene>();
        auto root = EntityFactory::CreateTransformEntity(*scene, "tile_map_test_scene");
        scene->m_root = root;

        auto id = EntityFactory::CreateTileMapEntity(*scene, "tile_map");
        scene->AttachChild(id);

        TileMapRenderer* tile_map_renderer = scene->GetComponent<TileMapRenderer>(id);
        tile_map_renderer->SetTileMap(p_guid);
        return scene;
    });
}

void TileMapEditor::OnDestroy() {
    m_tmp_scene = nullptr;  // decrease ref count
}

void TileMapEditor::OnActivate() {
    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);
    scene_manager->SetTmpScene(m_tmp_scene);
}

void TileMapEditor::DrawMainView() {
    ViewerTab::DrawMainView();

    const CameraComponent& camera = GetActiveCamera();
    const Matrix4x4f proj_view = camera.GetProjectionViewMatrix();

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
    SpriteAsset* sprite = tile_map->GetSpriteHandle().Get();

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
                if (sprite) {
                    EditSprite(*sprite);
                }
            },
        },
        {
            "PaintTab",
            0,
            [&]() {
                if (sprite) {
                    TilePaint(*sprite);
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
                m_document->RequestAdd(e->GetPos(), TileId(1));
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

    auto checkerboard = m_checkerboard_handle.Get();
    DEV_ASSERT(checkerboard);

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

        ImVec2 region_size(128, 128);
        ImVec2 image_size = region_size;

        uint64_t image_handle = 0;

        if (auto sprite = layer.GetSpriteHandle().Get(); sprite) {
            if (auto image = sprite->GetHandle().Get(); image) {
                image_handle = image->gpu_texture ? image->gpu_texture->GetHandle() : 0;
                image_size = ImVec2(static_cast<float>(image->width),
                                    static_cast<float>(image->height));
            }
        }
        if (image_handle == 0) {
            image_handle = checkerboard->gpu_texture->GetHandle();
        }

        CenteredImage(image_handle, image_size, region_size);

        if (ImGui::IsItemClicked()) {
            // tool->SetActiveLayer(layer_id);
        }

        DragDropTarget(AssetType::Sprite, [&](AssetHandle& p_handle) {
            DEV_ASSERT(p_handle.GetMeta()->type == AssetType::Sprite);
            layer.SetSpriteGuid(p_handle.GetGuid());
        });

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

void TileMapEditor::TilePaint(SpriteAsset& p_sprite) {
    ImGui::Text("TileSet");

    auto& handle = p_sprite.GetHandle();
    if (!handle.IsReady()) {
        return;
    }

    const ImageAsset* image = handle.Get();
    DEV_ASSERT(image);

    const uint32_t width = p_sprite.GetWidth();
    const uint32_t height = p_sprite.GetHeight();
    if (!width || !height) {
        return;
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 tile_size((float)width, (float)height);

    ImGui::InvisibleButton("TileClickable", tile_size);  // enables interaction
    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();
    unused(hovered);
    unused(clicked);

    // Draw tileset
    draw_list->AddImage(
        image->gpu_texture->GetHandle(),
        cursor,
        cursor + tile_size,
        ImVec2(0, 0), ImVec2(1, 1),
        IM_COL32(255, 255, 255, 255));

    const int num_col = p_sprite.GetCol();
    const int num_row = p_sprite.GetRow();

    const float cell_w = static_cast<float>(width) / num_col;
    const float cell_h = static_cast<float>(height) / num_row;

    if (hovered && clicked) {
        ImVec2 mouse_pos = ImGui::GetMousePos();

        int local_x = (int)((mouse_pos.x - cursor.x) / cell_w);
        int local_y = (int)((mouse_pos.y - cursor.y) / cell_h);

        // Clamp to valid range
        if (local_x >= 0 && local_x < num_col && local_y >= 0 && local_y < num_row) {
            m_selected_x = local_x;
            m_selected_y = local_y;
        }
    }

    for (float dx = cell_w; dx < tile_size.x; dx += cell_w) {
        draw_list->AddLine(ImVec2(cursor.x + dx, cursor.y),
                           ImVec2(cursor.x + dx, cursor.y + tile_size.y),
                           IM_COL32(255, 255, 255, 255));
    }

    for (float dy = cell_h; dy < tile_size.y; dy += cell_h) {
        draw_list->AddLine(ImVec2(cursor.x, cursor.y + dy),
                           ImVec2(cursor.x + tile_size.x, cursor.y + dy),
                           IM_COL32(255, 255, 255, 255));
    }

    if (m_selected_x >= 0 && m_selected_y >= 0) {
        float cellW = tile_size.x / num_col;
        float cellH = tile_size.y / num_row;

        ImVec2 pMin = ImVec2(cursor.x + m_selected_x * cellW, cursor.y + m_selected_y * cellH);
        ImVec2 pMax = ImVec2(pMin.x + cellW, pMin.y + cellH);

        draw_list->AddRectFilled(pMin, pMax, IM_COL32(0, 255, 0, 100));  // green transparent overlay
        draw_list->AddRect(pMin, pMax, IM_COL32(0, 255, 0, 255));        // solid border

    } else {
    }
}

void TileMapEditor::EditSprite(SpriteAsset& p_sprite) {
    if (ImGui::BeginTabBar("TileSetModes")) {
        if (ImGui::BeginTabItem("Setup")) {
            int column = p_sprite.GetCol();
            if (ImGui::InputInt("column", &column)) {
                p_sprite.SetCol(column);
            }
            int row = p_sprite.GetRow();
            if (ImGui::InputInt("row", &row)) {
                p_sprite.SetRow(row);
            }

            float scale = p_sprite.GetScale();
            if (ImGui::InputFloat("scale", &scale)) {
                p_sprite.SetScale(scale);
            }

            // ImGui::Checkbox("Use Texture Region", &use_region);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Select")) {
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}
}  // namespace cave
