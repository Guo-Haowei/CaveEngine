#include "TileMapEditor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/display/DisplayService.h"

#include "editor/EditorState.h"
#include "editor/panels/AssetInspector.h"
#include "editor/services/DocumentService.h"
#include "editor/widgets/DragDrop.h"
#include "editor/widgets/Image.h"

// @TODO: remove
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/view/ViewManager.h"
#include "engine/private/runtime/assets/TileSetAsset.h"

#include "editor/edit/EditCmdBase.h"

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

    Vector2f point_os = cursor_ + app_services_.displayService().windowPos();
    auto res = pointToTile(point_os);
    if (res.is_none()) {
        return;
    }

    TileIndex tile = res.unwrap_unchecked();

    if (mode_ == Mode::Painting) {
        auto selections = ctx_.sprite_selector.GetSelections();
        if (!selections.empty()) {
            auto [x, y] = selections[0];
            if (x >= 0 && y >= 0) {
                TileMapAsset* tile_map = doc->handle<TileMapAsset>().Get();
                TileSetAsset* tile_set = tile_map->GetTileSetHandle().Get();
                uint32_t idx = y * tile_set->GetCol() + x;
                LOG_OK("TODO: add {} {}", tile.x, tile.y);
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

Option<TileIndex> TileMapEditor::pointToTile(math::Vector2f point_os) {
    if (!isVisible()) return None();

    const ViewRecord* view = view_manager_.resolve(view_id_);
    if (!view->display_rect_os.Contains(point_os.x, point_os.y)) {
        return None();
    }

    Vector2f ndc = view->screenToNDC(point_os);

    Matrix4x4f pv_inv = glm::inverse(camera_.GetProjectionViewMatrix());

    Vector4f pos = pv_inv * Vector4f(ndc, 0.0f, 1.0f);
    pos /= pos.w;

    TileIndex index;
    index.x = static_cast<int16_t>(std::floor(pos.x));
    index.y = static_cast<int16_t>(std::floor(pos.y));
    return Some(index);
}

// @TODO: move to somewhere else
class SetTileCommand : public EditCmdBase {
public:
    const char* Label() const override { return "SetTileCommand"; }

    bool Do(IDocument& doc) override;
    bool Undo(IDocument& doc) override;
};

#if 0
struct CommandAddTile {
    TileIndex tile;
    TileId id;
};

struct CommandEraseTile {
    TileIndex tile;
};

class TileMapDocument : public OldDocument {
    using Command = std::variant<CommandAddTile, CommandEraseTile>;

public:
    TileMapDocument(const Guid& p_guid, const TileMapEditor& p_tile_map_editor)
        : OldDocument(p_guid)
        , m_tile_map_editor(p_tile_map_editor) {}

    void RequestAdd(const Vector2f& p_cursor, const TileId& p_id);
    void RequestErase(const Vector2f& p_cursor);

    void FlushCommands();

private:
    const TileMapEditor& m_tile_map_editor;
    std::vector<Command> m_commands;
};

// @TODO: abstract brush class
class SetTileCommand : public UndoCommand {
public:
    bool Undo() override {
        return SetTile(m_old_tile);
    }

    bool Redo() override {
        return SetTile(m_new_tile);
    }

    static std::unique_ptr<SetTileCommand> AddTile(TileMapAsset& p_tile_map, TileIndex p_index, TileId p_tile);

    static std::unique_ptr<SetTileCommand> RemoveTile(TileMapAsset& p_tile_map, TileIndex p_index);

    bool MergeCommand(const UndoCommand* p_other) override;

    void SetHandle(Handle<TileMapAsset>&& p_handle) {
        m_handle = std::move(p_handle);
    }

private:
    bool SetTile(Option<TileId> p_new_tile);

    Handle<TileMapAsset> m_handle;
    TileIndex m_index;

    Option<TileId> m_old_tile{ None() };
    Option<TileId> m_new_tile{ None() };
};

std::unique_ptr<SetTileCommand> SetTileCommand::AddTile(TileMapAsset& p_tile_map, TileIndex p_index, TileId p_tile) {
    Option<TileId> old_tile = p_tile_map.GetTile(p_index);

    if (!p_tile_map.AddTile(p_index, p_tile)) {
        return nullptr;
    }

    p_tile_map.IncRevision();

    auto cmd = std::make_unique<SetTileCommand>();
    cmd->m_old_tile = old_tile;
    cmd->m_new_tile = Some(p_tile);
    cmd->m_index = p_index;
    return cmd;
}

std::unique_ptr<SetTileCommand> SetTileCommand::RemoveTile(TileMapAsset& p_tile_map, TileIndex p_index) {
    Option<TileId> old_tile = p_tile_map.GetTile(p_index);

    if (!p_tile_map.RemoveTile(p_index)) {
        return nullptr;
    }

    p_tile_map.IncRevision();

    auto cmd = std::make_unique<SetTileCommand>();
    cmd->m_old_tile = old_tile;
    cmd->m_new_tile = None();
    cmd->m_index = p_index;
    return cmd;
}

bool SetTileCommand::MergeCommand(const UndoCommand* p_other) {
    if (auto other = dynamic_cast<const SetTileCommand*>(p_other); other) {
        return other->m_index == m_index &&
               other->m_new_tile == m_new_tile &&
               other->m_old_tile == m_old_tile &&
               other->m_handle.GetGuid() == m_handle.GetGuid();
    }
    return false;
}

bool SetTileCommand::SetTile(Option<TileId> p_new_tile) {
    TileMapAsset* tile_map = m_handle.Get();
    if (!tile_map) {
        return false;
    }
    const bool ok = p_new_tile.is_none() ? tile_map->RemoveTile(m_index) : tile_map->AddTile(m_index, p_new_tile.unwrap_unchecked());
    DEV_ASSERT(ok);
    tile_map->IncRevision();
    return ok;
}

void TileMapDocument::RequestAdd(const Vector2f& p_cursor, const TileId& p_id) {
    TileIndex tile;
    if (m_tile_map_editor.CursorToTile(p_cursor, tile)) {
        m_commands.emplace_back(CommandAddTile{ tile, p_id });
    }
}

void TileMapDocument::RequestErase(const Vector2f& p_cursor) {
    TileIndex tile;
    if (m_tile_map_editor.CursorToTile(p_cursor, tile)) {
        m_commands.emplace_back(CommandEraseTile{ tile });
    }
}

void TileMapDocument::FlushCommands() {
    auto handle = GetHandle<TileMapAsset>();
    TileMapAsset* tile_map = handle.Get();
    DEV_ASSERT(tile_map);
    if (!tile_map) return;

    // process commands
    for (const auto& command : m_commands) {
        std::visit([&](auto&& p_cmd) {
            using T = std::decay_t<decltype(p_cmd)>;
            if constexpr (std::is_same_v<T, CommandAddTile>) {
                if (auto cmd = SetTileCommand::AddTile(*tile_map, p_cmd.tile, p_cmd.id); cmd) {
                    m_dirty = true;
                    cmd->SetHandle(std::move(handle));
                    m_undo_stack->Submit(std::move(cmd));
                }
            } else if constexpr (std::is_same_v<T, CommandEraseTile>) {
                if (auto cmd = SetTileCommand::RemoveTile(*tile_map, p_cmd.tile); cmd) {
                    m_dirty = true;
                    cmd->SetHandle(std::move(handle));
                    m_undo_stack->Submit(std::move(cmd));
                }
            }
        },
                   command);
    }

    m_commands.clear();
}
#endif

}  // namespace cave
