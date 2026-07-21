#include "TileMapLayerPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/inspector/PropertyEditors.h"
#include "editor/services/DragDropService.h"
#include "editor/services/EditorServices.h"
#include "editor/services/IconCache.h"
#include "editor/widgets/Image.h"
#include "editor/widgets/SpriteSelector.h"

namespace cave {

using namespace ::cave::math;

void TileMapLayerPanel::draw(TileSetAsset& tile_set) {
    if (ImGui::BeginTabBar("##TileSet")) {
        if (ImGui::BeginTabItem("Layer")) {

            auto handle = tile_set.handle();
            const int column = tile_set.col();
            const int row = tile_set.row();
            if (auto image = handle.get(); image) {
                m_sprite_selector.SelectSprite(*image, &column, &row);
            }

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

}  // namespace cave
