#pragma once
#include "editor/panels/Tab.h"
#include "editor/widgets/SpriteSelector.h"

namespace cave {

class TileSetEditor final : public Tab {
public:
    using Tab::Tab;

protected:
    void drawAssetInspector(IDocument& doc) override;

    SpriteSelector sprite_selector_{ SpriteSelector::SelectionMode::Single };
};

}  // namespace cave
