#pragma once
#include "cave/runtime/tile_map/TileMapAsset.h"

namespace cave {

struct DrawComponentCtx;

class TileMapLayerPanel {
public:
    void draw(TileMapAsset& tile_map, DrawComponentCtx& ctx);

    const TileMapLayer* selectedLayer(const TileMapAsset& tile_map);

    Option<int> selectedIndex() const { return m_selected_layer; }

private:
    Option<int> m_selected_layer;
};

}  // namespace cave
