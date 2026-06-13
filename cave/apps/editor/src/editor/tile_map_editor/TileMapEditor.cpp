#include "TileMapEditor.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"

#include "editor/EditorState.h"
#include "editor/widgets/DragDrop.h"
#include "editor/widgets/Image.h"

// @TODO: remove
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/view/ViewManager.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

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

    const KeyState& st = app_services_.inputService().keyState();
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


#endif

}  // namespace cave
