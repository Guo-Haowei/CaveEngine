#pragma once
#include "engine/assets/asset_handle.h"

namespace cave {

class SpriteSelector {
public:
    bool EditSprite(int* p_colomn, int* p_row);

    void SelectSprite(const ImageAsset& p_handle,
                      const int* p_colomn,
                      const int* p_row);

    std::vector<std::pair<uint16_t, uint16_t>> GetSelections() const;

private:
    int m_column = 1;
    int m_row = 1;
    float m_zoom = 1.0f;

    std::set<uint32_t> m_selections;
    bool m_is_selection_mode;
};

}  // namespace cave
