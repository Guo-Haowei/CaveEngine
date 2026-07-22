#pragma once
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
    void drawTileSource(TileSetAsset* tile_set);
    void drawPaint();
    void drawAtlas(TileSetAsset* tile_set, ImageAsset* image);

    EngineServices& m_engine_services;
    EditorServices& m_editor_services;

    AtlasWidget m_atlas_widget;
    AtlasSelection m_atlas_selection;
    ImTextureID m_checkerboard_texture = 0;
    // @TODO: fix this
    int m_mode = 0;
    int m_paint_property = 0;
};

}  // namespace cave
