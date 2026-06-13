#include "TileMapEditor.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"

#include "editor/EditorState.h"
#include "editor/panels/AssetInspector.h"
#include "editor/services/DocumentService.h"
#include "editor/widgets/DragDrop.h"
#include "editor/widgets/Image.h"

// @TODO: remove
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/view/ViewManager.h"
#include "engine/private/runtime/assets/TileSetAsset.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

namespace cave {

using namespace ::cave::math;

TileMapEditor::TileMapEditor(EditorState& editor,
                             DocId doc_id,
                             SceneId scene_id)
    : ViewTabBase(editor, doc_id, scene_id, ViewDimension::Dim2)
    , ctx_(editor.assetInspector().tileMapContext())
    , debug_id_(MakeDebugId(this))
{

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

    if (m_editor.IsPlaying()) {
        return false;
    }

    const KeyState& st = app_services_.inputService().keyState();
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

void TileMapEditor::applayEditorTool() {
    IDocument* doc = editor_services_.document().resolve(doc_id_);
    DEV_ASSERT(doc);

    if (mode_ == Mode::Painting) {
        auto selections = ctx_.sprite_selector.GetSelections();
        if (!selections.empty()) {
            auto [x, y] = selections[0];
            if (x >= 0 && y >= 0) {
                TileMapAsset* tile_map = doc->handle<TileMapAsset>().Get();
                TileSetAsset* tile_set = tile_map->GetTileSetHandle().Get();
                uint32_t idx = y * tile_set->GetCol() + x;
                LOG_OK("TODO: add {} {}", x, y);
                unused(idx);
                // m_document->RequestAdd(e->GetPos(), TileId(idx));
            }
        }
    } else if (mode_ == Mode::Erasing) {
        LOG_OK("TODO: erase");
        // @TODO: earse tile
    }
}

void TileMapEditor::onInputEvents(const InputFrame& input) {
    if (!canHandleInput(input)) {
        return;
    }

    camera_controller_->Update(input);

    const bool should_apply_edit = updateEditMode(input);
    if (should_apply_edit) {
        applayEditorTool();
    }
}

void TileMapEditor::drawUIImpl() {
    ViewRecord* view = view_manager_.resolve(view_id_);
    DEV_ASSERT(view);

    updateRect(view->display_rect_os);
    drawMainView(view->display_rect_os);

    submitView();
}

#if 0
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
#endif

}  // namespace cave
