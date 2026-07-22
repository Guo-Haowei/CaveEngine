#pragma once
#include "editor/widgets/AtlasWidget.h"

namespace cave {

class TileSetPanel {
public:
    void draw();

private:
    AtlasWidget m_atlas_widget;
    AtlasSelection m_atlas_selection;
    ImTextureID m_checkerboard_texture = 0;
};

}  // namespace cave
