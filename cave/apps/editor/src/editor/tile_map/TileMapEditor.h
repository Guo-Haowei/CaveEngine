#pragma once
#include "cave/runtime/tile_map/TileMapAsset.h"

#include "editor/panels/ViewTabBase.h"
#include "editor/tile_map/GridPaintDefines.h"
#include "editor/widgets/SpriteSelector.h"

namespace cave {

enum class GridPaintAction : uint8_t;

struct GridPaintEvent;
class GridPaintTool;
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
    ~TileMapEditor();

    void onInputEvents(const InputFrame& input) override;

    DebugId debugId() const override { return m_debug_id; }

private:
    struct PendingChange {
        Option<TileId> before;
        Option<TileId> after;
    };

    void tileMapLayerOverview(TileMapAsset& tile_map);

    void drawUIImpl() override;
    void drawGizmo(const math::FloatRect& rect);
    void drawAssetInspector(IDocument& doc) override;
    void drawGhostTiles(const TileSetAsset& tile_set);

    void submitView();

    Option<TileCoord> pointToTile(math::Vec2f point_os);

    // ---- Paint Tool ----
    GridPaintInput buildInput(const InputFrame& input);
    void handlePaintEvent(const GridPaintEvent& event,
                          const TileMapAsset& tile_map,
                          const TileSetAsset& tile_set);

    void beginPaintCommand();
    void finishPaintCommand();
    void cancelPaintCommand();

    void applyPaintCells(std::span<const GridPaintCell> cells,
                         GridPaintAction action,
                         const TileMapAsset& tile_map,
                         const TileSetAsset& tile_set);
    // ---- Paint Tool ----

    ICanvas& m_canvas;
    Owner<GridPaintTool> m_paint_tool;
    const DebugId m_debug_id;

    HashMap<TileCoord, PendingChange> m_pending_tile_changes;
    Option<math::Vec2f> m_cursor;

    // @TODO: review this part
    SpriteSelector m_sprite_selector{ SpriteSelector::SelectionMode::Single };
};

}  // namespace cave
