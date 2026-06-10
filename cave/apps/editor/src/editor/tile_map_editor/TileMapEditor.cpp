#include "TileMapEditor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/diagnostics/DebugIdAllocator.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/assets/TileSetAsset.h"
#include "engine/private/runtime/input/InputService.h"

#include "editor/EditorState.h"
#include "editor/widgets/DragDrop.h"
#include "editor/widgets/Image.h"
#include "engine/private/ui/inputs.h"
#include "engine/private/ui/layout.h"

// @TODO: remove
#include "engine/private/runtime/view/ViewManager.h"

namespace cave {

using namespace ::cave::math;

TileMapEditor::TileMapEditor(EditorState& editor,
                             DocId doc_id,
                             SceneId scene_id)
    : ViewTabBase(editor, doc_id, scene_id, ViewDimension::Dim2)
    , debug_id_(MakeDebugId(this))
    , sprite_selector_(SpriteSelector::SelectionMode::Single) {

    // m_brush_desc = ToolBarButtonDesc{ ICON_FA_BRUSH, "TileMap editor mode",
    //                                   [&]() {
    //                                       LOG_WARN("TODO");
    //                                   } };

    // @TODO: use Intent for editing tiles?
}

void TileMapEditor::submitView() {
    ViewTabBase::submitView(false);
}

void TileMapEditor::onCreate() {
    ViewTabBase::onCreate();
}

void TileMapEditor::onDestroy() {
    ViewTabBase::onDestroy();
}

Option<PickData> TileMapEditor::getPickData(const Vector2f& pointer_os) {
    unused(pointer_os);

    return None();

    // if (!IsVisible()) return None();

    // const ViewRecord* view = view_manager_.resolve(view_id_);
    // if (!view->display_rect_os.Contains(pointer_os.x, pointer_os.y)) {
    //     return None();
    // }

    // return Some(PickData{
    //     .proj_view = camera_.GetProjectionViewMatrix(),
    //     .cursor_ndc = view->screenToNDC(pointer_os),
    //     .scene_id = preview_scene_id_,
    //     .doc_id = doc_id_,
    // });
}

void TileMapEditor::onInputEvents(const InputFrame& input) {
    if (!isHovered()) {
        return;
    }

    if (m_editor.IsPlaying()) {
        return;
    }

    const KeyState& st = services_.inputService().keyState();
    if (st.anyAltDown() || st.anyCtrlDown() || st.anyShiftDown()) {
        return;
    }

    camera_controller_->Update(input);
}

void TileMapEditor::drawUIImpl() {
    ViewRecord* view = view_manager_.resolve(view_id_);
    DEV_ASSERT(view);

    updateRect(view->display_rect_os);
    drawMainView(view->display_rect_os);

    submitView();
}

#if 0
void TileMapEditor::DrawAssetInspector() {
    TileMapAsset* tile_map = m_document->GetHandle<TileMapAsset>().Get();
    TileSetAsset* tile_set = tile_map->GetTileSetHandle().Get();

    std::vector<AssetChildPanel> descs = {
        {
            "LayerOverview",
            720,
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

    ui::DrawContents(full_width, descs);
}

bool TileMapEditor::CursorToTile(const Vector2f& p_in, TileIndex& p_out) const {
    auto res = m_viewer.CursorToNDC(p_in);
    if (res.is_none()) {
        return false;
    }

    auto ndc_2 = res.unwrap_unchecked();
    Vector4f ndc{ ndc_2.x, ndc_2.y, 0.0f, 1.0f };

    DEV_ASSERT(0);
    CameraComponent cam;
    const auto inv_proj_view = glm::inverse(cam.GetProjectionViewMatrix());

    Vector4f position = inv_proj_view * ndc;
    position /= position.w;

    p_out.x = static_cast<int16_t>(std::floor(position.x));
    p_out.y = static_cast<int16_t>(std::floor(position.y));

    return true;
}

bool TileMapEditor::HandleInput(const OldInputEvent* p_input_event) {
    DEV_ASSERT(0);
    unused(p_input_event);
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

        if (ui::TextBox("layer", layer.GetName())) {
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

            const ImageAsset* image = nullptr;
            if (auto image_handle = layer.GetTileSetHandle().Get(); image_handle) {
                image = image_handle->GetHandle().Get();
            }

            auto checkerboard = m_editor.context.checkerboard;
            DEV_ASSERT(checkerboard && checkerboard->gpu_texture);

            Vector2f region_size(128, 128);
            ui::CenteredImage(image, region_size, checkerboard->gpu_texture->GetHandle());

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
#endif

}  // namespace cave
