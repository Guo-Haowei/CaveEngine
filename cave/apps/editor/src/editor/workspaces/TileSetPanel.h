#pragma once
#include "TileSetPanel/TileTerrainPaintTool.h"
#include "TileSetPanel/TilePhysicsPaintTool.h"

namespace cave {

struct EngineServices;
struct EditorServices;
struct ImageAsset;
struct SceneEditContext;
class TileSetAsset;

class TileSetPanel {
public:
    explicit TileSetPanel(EngineServices& engine_services,
                          EditorServices& editor_services);

    void draw(SceneEditContext* context);

private:
    bool drawTileSource(TileSetAsset* tile_set);
    bool drawTileProperties(TileSetAsset* tile_set);
    bool drawAtlas(TileSetAsset* tile_set, ImageAsset* image);

    void drawPaintPropertySelector();

    bool handleAtlasPainting(TileSetAsset& tile_set,
                             const AtlasWidgetResult& result,
                             const ImageCanvasInput& input);

    void drawAtlasMetadataOverlays(const TileSetAsset* tile_set,
                                   const AtlasLayout& layout,
                                   const AtlasWidgetResult& result);

    void drawHoveredTerrainCell(const AtlasLayout& layout,
                                const AtlasHit& hit,
                                const AtlasWidgetResult& result);

    bool drawTileContextPopup(TileSetAsset& tile_set);

    EngineServices& m_engine_services;
    EditorServices& m_editor_services;

    AtlasWidget m_atlas_widget;
    ImTextureID m_checkerboard = 0;

    enum class Property {
        Setup,
        SelectedTile,
        Paint
    } m_mode = Property::Setup;

    enum class PaintProperty : int {
        Physics = 0,
        Terrain,
        Animation,
    };

    PaintProperty m_paint_property = PaintProperty::Terrain;

    Option<uint32_t> m_context_tile;

    Option<uint32_t> m_last_painted_tile;

    TileTerrainPaintTool m_terrain_paint_tool;
    TilePhysicsPaintTool m_physics_paint_tool;
};

}  // namespace cave
