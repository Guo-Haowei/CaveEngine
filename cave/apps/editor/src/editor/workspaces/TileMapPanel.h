#pragma once
#include "editor/widgets/SpriteSelector.h"
#include "editor/widgets/ToolBar.h"

namespace cave {

enum class GridPaintMode : uint8_t;
struct SceneEditContext;

class TileMapPanel {
public:
    explicit TileMapPanel();

    void draw(SceneEditContext* context);

private:
    void setPaintMode(GridPaintMode mode);

    SpriteSelector m_sprite_selector{ SpriteSelector::SelectionMode::Single };

    GridPaintMode m_paint_mode;
    std::array<ToolbarButtonDesc, 4> m_toolbar;
};

}  // namespace cave
