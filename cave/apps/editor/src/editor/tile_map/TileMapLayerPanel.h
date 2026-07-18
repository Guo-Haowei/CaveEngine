#pragma once
#include "cave/runtime/tile_map/TileMapAsset.h"

namespace cave {

struct DrawComponentCtx;
class SpriteSelector;

class TileMapLayerPanel {
public:
    TileMapLayerPanel(SpriteSelector& sprite_selector) noexcept
        : m_sprite_selector(sprite_selector) {}

    void draw(TileMapAsset& tile_map, DrawComponentCtx& ctx);

    const TileMapLayer* selectedLayer(const TileMapAsset& tile_map);

    Option<int> selectedIndex() const { return m_selected_layer; }

private:
    void drawLayers(TileMapAsset& tile_map, DrawComponentCtx& ctx);

    SpriteSelector& m_sprite_selector;
    Option<int> m_selected_layer;
};

}  // namespace cave
