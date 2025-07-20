#pragma once
#include "engine/assets/asset_handle.h"

namespace cave {

class SpriteSelector {
public:
    void EditSprite();
    void SelectSprite(const ImageAsset& p_handle);

    int m_column = 1;
    int m_row = 1;

    int m_selected_x = -1;
    int m_selected_y = -1;

};

}  // namespace cave
