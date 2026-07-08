#include "TileMapEditor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/display/DisplayService.h"
#include "cave/runtime/display/IDebugDrawService.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/EditorState.h"
#include "editor/panels/AssetInspector.h"
#include "editor/services/EditService.h"
#include "editor/services/DocumentService.h"
#include "editor/services/IconCache.h"
#include "editor/tile_map/SetTileCommand.h"
#include "editor/widgets/DragDrop.h"
#include "editor/widgets/Image.h"

// @TODO: remove
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/view/ViewManager.h"
#include "editor/utility/ImGuizmo.h"

namespace cave {

using namespace ::cave::math;

TileMapEditor::TileMapEditor(EditorState& editor,
                             DocId doc_id,
                             SceneId scene_id)
    : ViewTabBase(editor, doc_id, scene_id, ViewDimension::Dim2)
    , debug_id_(MakeDebugId(this)) {

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

void TileMapEditor::changeMode(Mode mode) {
    if (mode != mode_) {
        // LOG_INFO("change mode from {} to {}", (int)mode_, (int)mode);
        mode_ = mode;
    }
}

bool TileMapEditor::canHandleInput(const InputFrame& input) {
    unused(input);

    if (!isHovered()) {
        return false;
    }

    const KeyState& st = m_engine_services.inputService().keyState();
    if (st.anyAltDown() || st.anyCtrlDown() || st.anyShiftDown()) {
        return false;
    }

    return true;
}

bool TileMapEditor::updateEditMode(const InputFrame& input) {
    bool should_apply = false;

    for (const InputEvent& event : input.events) {
        Key key = static_cast<Key>(event.code);
        switch (event.type) {
            case InputEventType::ButtonDown: {
                if (key == Key::LMB) {
                    lb_down_ = true;
                    event.consumed = true;
                    should_apply = true;
                    cursor_ = { event.x, event.y };
                } else if (key == Key::RMB) {
                    rb_down_ = true;
                    event.consumed = true;
                    should_apply = true;
                    cursor_ = { event.x, event.y };
                }
            } break;
            case InputEventType::ButtonUp: {
                if (key == Key::LMB) {
                    lb_down_ = false;
                    event.consumed = true;
                } else if (key == Key::RMB) {
                    rb_down_ = false;
                    event.consumed = true;
                }
            } break;
            case InputEventType::MouseMove: {
                should_apply = true;
                cursor_ = { event.x, event.y };
            } break;
            default: {
            } break;
        }
    }

    if (!(lb_down_ ^ rb_down_))
        changeMode(Mode::None);
    else if (lb_down_)
        changeMode(Mode::Painting);
    else if (rb_down_)
        changeMode(Mode::Erasing);

    return should_apply && mode_ != Mode::None;
}

void TileMapEditor::updateTileCoord() {
    Vec2f point_os = cursor_ + m_engine_services.displayService().windowPos();
    auto res = pointToTile(point_os);
    if (res.is_none()) {
        return;
    }

    coord_ = res.unwrap_unchecked();

    Vec2f min{ coord_.x, coord_.y };
    Vec2f max{ coord_.x + 1, coord_.y + 1 };
    m_engine_services.debugDraw().addBox2(min, max, Vec4f{ 0.7f, 0.2f, 0.2f, 0.7f });
}

void TileMapEditor::applayEditorTool() {
    IDocument* doc = m_editor_services.document().resolve(m_doc_id);
    DEV_ASSERT(doc);

    TileMapAsset* tile_map = doc->handle<TileMapAsset>().get();

    Option<TileId> old_tile = tile_map->tiles().tileAt(coord_);
    Option<TileId> new_tile = None();

    switch (mode_) {
        case cave::TileMapEditor::Mode::None:
            return;
        case cave::TileMapEditor::Mode::Painting: {
            auto selections = sprite_selector_.GetSelections();
            if (selections.empty()) {
                return;
            }
            auto [x, y] = selections[0];
            if (x >= 0 && y >= 0) {
                TileSetAsset* tile_set = tile_map->tileSetHandle().get();
                const uint32_t tile_id = y * tile_set->col() + x;
                new_tile = Some(TileId(tile_id));
            }
        } break;
        case cave::TileMapEditor::Mode::Erasing: {
            // old tile is already None
            if (old_tile.unwrap_or(kEmptyTileId) == kEmptyTileId) {
                return;
            }
        } break;
    }

    if (old_tile == new_tile) {
        return;  // no op if the tiles are the same
    }

    auto cmd = std::make_unique<SetTileCommand>(m_engine_services.sceneRegistry(),
                                                ecs::Entity::null(),
                                                coord_,
                                                old_tile,
                                                new_tile);
    m_editor_services.edit().submit(m_doc_id, std::move(cmd));
}

void TileMapEditor::onInputEvents(const InputFrame& input) {
    if (!canHandleInput(input)) {
        return;
    }

    m_camera_controller->update(input);

    const bool should_apply_edit = updateEditMode(input);
    updateTileCoord();
    if (should_apply_edit) {
        applayEditorTool();
    }
}

void TileMapEditor::drawGizmo(const math::FloatRect& rect) {

    const Mat4f& proj_view = m_camera.projectionViewMatrix();

    ImGuizmo::SetOrthographic(true);
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(rect.x, rect.y, rect.w, rect.h);

    ImGuizmo::DrawGrid(proj_view, Mat4f(1.0f), 100.0f, ImGuizmo::GridPlane::XY);
}

void TileMapEditor::drawUIImpl() {
    ViewRecord* view = m_view_manager.resolve(m_view_id);
    DEV_ASSERT(view);

    updateRect(view->display_rect_os);
    drawMainView(view->display_rect_os);

    drawGizmo(view->display_rect_os);

    submitView();
}

void TileMapEditor::drawAssetInspector(IDocument& doc) {
    TileMapAsset* tile_map = doc.handle<TileMapAsset>().get();
    DEV_ASSERT(tile_map);

    if (ImGui::BeginTabBar("##MyTabs1")) {
        if (ImGui::BeginTabItem("Layer")) {
            tileMapLayerOverview(*tile_map);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();

    TileSetAsset* tile_set = tile_map->tileSetHandle().get();
    if (tile_set) {
        auto handle = tile_set->handle();
        const int column = tile_set->col();
        const int row = tile_set->row();
        if (auto image = handle.get(); image) {
            sprite_selector_.SelectSprite(*image, &column, &row);
        }
    }
}

void TileMapEditor::tileMapLayerOverview(TileMapAsset& tile_map) {
    if (ImGui::Button(ICON_FA_SQUARE_PLUS " Add Layer")) {
        // p_tile_map.AddLayer("untitled layer");
        LOG_WARN("TODO: Add layer");
    }
    ImGui::Separator();

    for (int layer_id = 0; layer_id < 1; ++layer_id) {
        TileMapAsset& layer = tile_map;
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
            if (auto image_handle = layer.tileSetHandle().get(); image_handle) {
                image = image_handle->handle().get();
            }

            Vec2f region_size(128, 128);
            IconCache& icons = m_editor_services.iconCache();
            ui::CenteredImage(image, region_size, icons.GetIconHandle(IconName::Checkerboard));

            if (ImGui::IsItemClicked()) {
                // tool->SetActiveLayer(layer_id);
            }

            // @TODO: make an asset drop region
            // accept same type of assets, show tooltips, etc
            if (auto _handle = DragDropTarget(AssetType::TileSet)) {
                layer.tileSetGuid(_handle.unwrap_unchecked().guid());
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
Option<TileCoord> TileMapEditor::pointToTile(math::Vec2f point_os) {
    if (!isVisible()) return None();

    const ViewRecord* view = m_view_manager.resolve(m_view_id);
    if (!view->display_rect_os.Contains(point_os.x, point_os.y)) {
        return None();
    }

    Vec2f ndc = view->screenToNDC(point_os);

    Mat4f pv_inv = glm::inverse(m_camera.projectionViewMatrix());

    Vec4f pos = pv_inv * Vec4f(ndc, 0.0f, 1.0f);
    pos /= pos.w;

    TileCoord index;
    index.x = static_cast<int16_t>(std::floor(pos.x));
    index.y = static_cast<int16_t>(std::floor(pos.y));
    return Some(index);
}

}  // namespace cave
