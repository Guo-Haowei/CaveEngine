#pragma once
#include "cave/runtime/tile_map/TileMapAsset.h"

#include "editor/panels/ViewTabBase.h"
#include "editor/widgets/SpriteSelector.h"

namespace cave {

class ICanvas;

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

    DebugId debugId() const override { return m_debug_id; }

private:
    void drawUIImpl() override;
    void drawGizmo(const math::FloatRect& rect);
    void drawAssetInspector(IDocument& doc) override;
    void tileMapLayerOverview(TileMapAsset& tile_map);

    void submitView();
    void changeMode(Mode mode);
    bool canHandleInput(const InputFrame& input);
    bool updateEditMode(const InputFrame& input);
    void updateTileCoord();

    void drawOverlay(const TileSetAsset& tile_set);
    void applayEditorTool(const TileMapAsset& tile_map,
                          const TileSetAsset& tile_set);

    Option<TileCoord> pointToTile(math::Vec2f point_os);

    ICanvas& m_canvas;
    const DebugId m_debug_id;

    SpriteSelector m_sprite_selector{ SpriteSelector::SelectionMode::Single };
    Mode m_mode{ Mode::None };
    bool m_lb_down{ false };
    bool m_rb_down{ false };
    math::Vec2f m_cursor;
    TileCoord m_coord;
};

}  // namespace cave
