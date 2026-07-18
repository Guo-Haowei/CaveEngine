#pragma once

namespace cave {

struct DrawComponentCtx;

class TileMapAsset;
class IconCache;

class TileMapLayerPanel {
public:
    void draw(TileMapAsset& tile_map, DrawComponentCtx& ctx);

private:
    Option<int> m_selected_layer;
};

}  // namespace cave
