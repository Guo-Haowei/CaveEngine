#pragma once
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/widgets/AtlasWidget.h"

namespace cave {

class TileTerrainPaintTool {
public:
    bool drawPaintProperties(TileSetAsset* tile_set);

    void drawOverlay(const TileDefinition& definition,
                     const AtlasLayout& layout,
                     const AtlasWidgetResult& result);

    bool handleAtlasPainting(TileSetAsset& tile_set,
                             const AtlasWidgetResult& result,
                             const ImageCanvasInput& input);

private:
    bool paintMask(TileSetAsset& tile_set,
                   const AtlasHit& hit,
                   bool erase);

    void drawMaskOverlay(const TileDefinition& definition,
                         const Box2& tile_rect,
                         const AtlasLayout& layout,
                         const AtlasWidgetResult& result);

    Option<uint32_t> m_last_painted_mask_cell;
    TerrainId m_terrain_id = TerrainId(0);
};

}  // namespace cave
