#include "TileMapEditor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/display/DisplayService.h"

#include "editor/EditorState.h"
#include "editor/panels/AssetInspector.h"
#include "editor/services/EditService.h"
#include "editor/services/DocumentService.h"
#include "editor/widgets/DragDrop.h"
#include "editor/widgets/Image.h"

// @TODO: remove
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/view/ViewManager.h"
#include "engine/private/runtime/assets/TileSetAsset.h"

namespace cave {

using namespace ::cave::math;

TileMapEditor::TileMapEditor(EditorState& editor,
                             DocId doc_id,
                             SceneId scene_id)
    : ViewTabBase(editor, doc_id, scene_id, ViewDimension::Dim2)
    , ctx_(editor.assetInspector().tileMapContext())
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

    Vec2f point_os = cursor_ + app_services_.displayService().windowPos();
    auto res = pointToTile(point_os);
    if (res.is_none()) {
        return;
    }

    TileCoord tile_index = res.unwrap_unchecked();

    TileMapAsset* tile_map = doc->handle<TileMapAsset>().Get();

    Option<TileId> old_tile = tile_map->tileAt(tile_index);
    Option<TileId> new_tile = Some(kEmptyTileId);

    if (mode_ == Mode::Painting) {
        auto selections = ctx_.sprite_selector.GetSelections();
        if (!selections.empty()) {
            auto [x, y] = selections[0];
            if (x >= 0 && y >= 0) {
                TileSetAsset* tile_set = tile_map->tileSetHandle().Get();
                const uint32_t tile_id = y * tile_set->GetCol() + x;
                new_tile = Some(TileId(tile_id));
            }
        }
    }

    if (old_tile == new_tile) {
        return;  // no op if the tiles are the same
    }

    auto cmd = std::make_unique<SetTileCommand>(app_services_.sceneRegistry(),
                                                ecs::Entity::Null(),
                                                tile_index,
                                                old_tile,
                                                new_tile);
    editor_services_.edit().submit(doc_id_, std::move(cmd));
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

Option<TileCoord> TileMapEditor::pointToTile(math::Vec2f point_os) {
    if (!isVisible()) return None();

    const ViewRecord* view = view_manager_.resolve(view_id_);
    if (!view->display_rect_os.Contains(point_os.x, point_os.y)) {
        return None();
    }

    Vec2f ndc = view->screenToNDC(point_os);

    Matrix4x4f pv_inv = glm::inverse(camera_.GetProjectionViewMatrix());

    Vec4f pos = pv_inv * Vec4f(ndc, 0.0f, 1.0f);
    pos /= pos.w;

    TileCoord index;
    index.x = static_cast<int16_t>(std::floor(pos.x));
    index.y = static_cast<int16_t>(std::floor(pos.y));
    return Some(index);
}

}  // namespace cave
