#pragma once
#include "editor/widgets/SpriteSelector.h"

namespace cave {

struct TileMapEditorContext {
    SpriteSelector sprite_selector{ SpriteSelector::SelectionMode::Single };
};

}  // namespace cave
