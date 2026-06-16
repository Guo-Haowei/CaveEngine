#pragma once
#include "cave/runtime/assets/TileMapAsset.h"
#include "cave/runtime/view/ViewDesc.h"

#include "editor/document/SceneDocument.h"
#include "editor/panels/ViewTabBase.h"
#include "editor/services/IPickConsumer.h"
#include "editor/tile_map_editor/SetTileCommand.h"
#include "editor/tile_map_editor/TileMapEditorContext.h"

namespace cave {

class TileMapEditor final : public ViewTabBase {
    enum class Mode : uint8_t {
        None,
        Painting,
        Erasing,
    };

public:
    TileMapEditor(EditorState& editor,
                  DocId doc_id,
                  SceneId preview_scene_id);

    void onCreate() override;
    void onDestroy() override;

    void onInputEvents(const InputFrame& input) override;

    DebugId debugId() const override { return debug_id_; }

protected:
    void drawUIImpl() override;

    void submitView();
    void changeMode(Mode mode);
    bool canHandleInput(const InputFrame& input);
    bool updateEditMode(const InputFrame& input);
    void applayEditorTool();
    Option<TileIndex> pointToTile(math::Vec2f point_os);

    TileMapEditorContext& ctx_;
    const DebugId debug_id_;

    Mode mode_{ Mode::None };
    bool lb_down_{ false };
    bool rb_down_{ false };
    math::Vec2f cursor_;
};

}  // namespace cave
