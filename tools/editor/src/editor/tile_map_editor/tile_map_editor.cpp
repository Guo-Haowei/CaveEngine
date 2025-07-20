#include "tile_map_editor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/assets/image_asset.h"
#include "engine/input/input_event.h"
#include "engine/scene/entity_factory.h"
#include "editor/editor_layer.h"
#include "editor/editor_scene_manager.h"
#include "editor/widgets/widget.h"
#include "editor/viewer/viewer.h"
#include "editor/utility/imguizmo.h"
#include "editor/tile_map_editor/tile_map_document.h"

// @TODO: refactor
#include "engine/assets/assets.h"
#include "engine/assets/sprite_asset.h"

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
    m_tmp_scene.reset();
}

void TileMapEditor::OnActivate() {
    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);
    scene_manager->SetTmpScene(m_tmp_scene);
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
                m_sprite_selector.EditSprite();
            },
        },
        {
            "PaintTab",
            0,
            [&]() {
                if (sprite) {
                    auto handle = sprite->GetHandle();
                    if (auto image = handle.Get(); image) {
                        m_sprite_selector.SelectSprite(*image);
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

        /// @TODO: generalize
        ImVec2 region_size(128, 128);
        ImVec2 image_size = region_size;

        uint64_t texture_handle = 0;

        if (auto sprite = layer.GetSpriteHandle().Get(); sprite) {
            if (auto image = sprite->GetHandle().Get(); image) {
                texture_handle = image->gpu_texture ? image->gpu_texture->GetHandle() : 0;
                image_size = ImVec2(static_cast<float>(image->width),
                                    static_cast<float>(image->height));
            }
        }
        if (texture_handle == 0) {
            auto checkerboard = m_editor.context.checkerboard_handle.Get();
            DEV_ASSERT(checkerboard);
            texture_handle = checkerboard->gpu_texture->GetHandle();
        }

        CenteredImage(texture_handle, image_size, region_size);

        if (ImGui::IsItemClicked()) {
            // tool->SetActiveLayer(layer_id);
        }

        DragDropTarget(AssetType::Sprite, [&](AssetHandle& p_handle) {
            DEV_ASSERT(p_handle.GetMeta()->type == AssetType::Sprite);
            layer.SetSpriteGuid(p_handle.GetGuid());
        });
        /// @TODO: generalize

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

const std::vector<ViewerTab::ToolBarButtonDesc>& TileMapEditor::GetToolBarButtons() const {
    static std::vector<ToolBarButtonDesc> s_buttons = {
        { ICON_FA_BRUSH, "TileMap editor mode",
          [&]() {
              LOG_WARN("TODO");
          } },
    };

    return s_buttons;
}

}  // namespace cave
