#pragma once
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/widgets/AtlasWidget.h"

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
    bool drawTerrainPaintProperties(TileSetAsset* tile_set);
    bool paintTerrain(TileSetAsset& tile_set, const AtlasHit& hit);

    bool drawPhysicsPaintProperties(TileSetAsset* tile_set);
    bool drawPhysicsShapeEditor(math::Box2& shape);
    bool paintPhysics(TileSetAsset& tile_set, const AtlasHit& hit);

    bool handleAtlasPainting(TileSetAsset& tile_set, const AtlasWidgetResult& result);
    bool paintAtlasTile(TileSetAsset& tile_set, const AtlasHit& hit);

    void drawAtlasMetadataOverlays(const TileSetAsset* tile_set,
                                   const AtlasLayout& layout,
                                   const AtlasWidgetResult& result);
    void drawPhysicsOverlay(const TileDefinition& definition,
                            const AtlasLayout& layout,
                            const AtlasWidgetResult& result);
    void drawTerrainOverlay(const TileDefinition& definition,
                            const AtlasLayout& layout,
                            const AtlasWidgetResult& result);
    void drawTerrainMaskOverlay(const TileDefinition& definition,
                                const Box2& tile_rect,
                                const AtlasLayout& layout,
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

    enum class PhysicsPaintTool {
        Assign,
        Remove,
    };

    struct TerrainPaintState {
        int terrain_id = 0;

        // 3x3 mask:
        //
        // 0 1 2
        // 3 4 5
        // 6 7 8
        //
        uint16_t mask = 1u << 4;

        bool erase = false;
    };

    struct PhysicsPaintState {
        PhysicsPaintTool tool = PhysicsPaintTool::Assign;

        // Normalized tile-local AABB.
        Box2 shape{
            Vec2f{ 0.0f, 0.0f },
            Vec2f{ 1.0f, 1.0f },
        };

        CollisionType collision = CollisionType::Solid;
        uint32_t mask = 1;

        // Interaction with the small physics preview.
        bool dragging = false;
        int drag_handle = -1;
    };

    PaintProperty m_paint_property = PaintProperty::Physics;

    TerrainPaintState m_terrain_paint;
    PhysicsPaintState m_physics_paint;

    Option<uint32_t> m_last_painted_tile;
    Option<uint32_t> m_context_tile;
};

}  // namespace cave
