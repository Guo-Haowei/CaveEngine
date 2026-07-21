#pragma once
#include "cave/runtime/tile_map/TileMapLayerComponent.h"

#include "editor/scene_view/ISceneViewTool.h"
#include "editor/tile_map/GridPaintDefines.h"
#include "editor/tile_map/GridPaintTool.h"
#include "editor/widgets/SpriteSelector.h"

namespace cave {

enum class GridPaintAction : uint8_t;

struct GridPaintEvent;
class GridPaintTool;
class ICanvas;
class TileMapLayerPanel;

class TilePaintTool : public ISceneViewTool {
    enum class Mode : uint8_t {
        None,
        Painting,
        Erasing,
    };

public:
    TilePaintTool(const SceneToolContext& ctx);
    ~TilePaintTool() override;

    void onInputEvents(const InputFrame& input, const WindowState& state) override;

    void draw(const math::FloatRect& rect) override;

    void drawAssetInspector(IDocument& doc) override;

    void setLayerId(ecs::Entity id) { m_layer_id = id; }

private:
    struct PendingChange {
        Option<TileId> before;
        Option<TileId> after;
    };

    void drawGhostTiles(const TileSetAsset& tile_set);

    Option<TileCoord> pointToTile(math::Vec2f point_os);

    // ---- Paint Tool ----
    GridPaintInput buildInput(const InputFrame& input, const WindowState& state);
    void handlePaintEvent(const GridPaintEvent& event,
                          const TileMapLayerComponent& layer);

    void beginPaintCommand();
    void finishPaintCommand();
    void cancelPaintCommand();

    void applyPaintCells(std::span<const GridPaintCell> cells,
                         const TileMapLayerComponent& layer);

    void applyFillCells(GridPaintCell cell, const TileMapLayerComponent& layer);

    void setPaintMode(GridPaintMode mode);
    // ---- Paint Tool ----

    TileSetAsset* getTileSet(ecs::Entity entity);
    TileMapLayerComponent* getTileMapLayer(ecs::Entity entity);

    GridPaintTool m_paint_tool;

    HashMap<TileCoord, PendingChange> m_pending_tile_changes;
    Option<math::Vec2f> m_cursor;

    GridPaintMode m_paint_mode = GridPaintMode::Brush;
    bool m_erasing = true;

    Owner<TileMapLayerPanel> m_tile_map_layer_panel;

    SpriteSelector m_sprite_selector{ SpriteSelector::SelectionMode::Single };

    ecs::Entity m_layer_id;
};

}  // namespace cave
