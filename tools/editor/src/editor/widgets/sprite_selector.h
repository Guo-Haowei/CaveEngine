#pragma once
#include "engine/assets/asset_handle.h"

namespace cave {

class SpriteSelector {
public:
    bool EditSprite(int* p_colomn, int* p_row);
    void SelectSprite(const ImageAsset& p_handle,
                      const int* p_colomn,
                      const int* p_row);

private:
    int m_column = 1;
    int m_row = 1;
    float m_zoom = 1.0f;

    int m_selected_x = -1;
    int m_selected_y = -1;
};

}  // namespace cave
