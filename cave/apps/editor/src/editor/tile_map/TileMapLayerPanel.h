#pragma once

namespace cave {

class SpriteSelector;
class TileSetAsset;

class TileMapLayerPanel {
public:
    TileMapLayerPanel(SpriteSelector& sprite_selector) noexcept
        : m_sprite_selector(sprite_selector) {}

    void draw(TileSetAsset& tile_set);

private:
    SpriteSelector& m_sprite_selector;
};

}  // namespace cave
