#pragma once
#include "cave/runtime/tile_map/TileMapAsset.h"

#include "editor/panels/ViewTabBase.h"
#include "editor/widgets/SpriteSelector.h"

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

private:
    void drawUIImpl() override;
    void drawAssetInspector(IDocument& doc) override;
    void tileMapLayerOverview(TileMapAsset& tile_map);

    void submitView();
    void changeMode(Mode mode);
    bool canHandleInput(const InputFrame& input);
    bool updateEditMode(const InputFrame& input);
    void updateTileCoord();
    void applayEditorTool();
    Option<TileCoord> pointToTile(math::Vec2f point_os);

    const DebugId debug_id_;

    SpriteSelector sprite_selector_{ SpriteSelector::SelectionMode::Single };
    Mode mode_{ Mode::None };
    bool lb_down_{ false };
    bool rb_down_{ false };
    math::Vec2f cursor_;
    TileCoord coord_;
};

}  // namespace cave
