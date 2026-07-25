#pragma once
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/widgets/AtlasWidget.h"

namespace cave {

class TilePhysicsPaintTool {
    enum class PhysicsPaintTool {
        Assign,
        Remove,
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

public:
    void drawOverlay(const TileDefinition& definition,
                     const AtlasLayout& layout,
                     const AtlasWidgetResult& result);

    bool drawPaintProperties(TileSetAsset* tile_set);

    bool handleAtlasPainting(TileSetAsset& tile_set,
                             const AtlasWidgetResult& result,
                             const ImageCanvasInput& input);

private:
    bool paintPhysics(TileSetAsset& tile_set, const AtlasHit& hit);
    bool drawPhysicsShapeEditor(math::Box2& shape);

    PhysicsPaintState m_physics_paint;
};

}  // namespace cave
