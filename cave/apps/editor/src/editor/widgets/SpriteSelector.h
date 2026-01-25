#pragma once
#include "engine/assets/asset_handle.h"

namespace cave {

class SpriteSelector {
public:
    enum class SelectionMode {
        Single,
        Multi,
    };

    SpriteSelector(SelectionMode p_mode)
        : m_mode(p_mode) {}

    bool EditSprite(int* p_colomn, int* p_row);

    void SelectSprite(const ImageAsset& p_handle,
                      const int* p_colomn,
                      const int* p_row);

    std::vector<std::pair<uint16_t, uint16_t>> GetSelections() const;

    void ClearSelections() {
        m_selections.clear();
    }

    std::pair<int, int> GetDim() const {
        return std::make_pair(m_column, m_row);
    }

private:
    const SelectionMode m_mode;

    int m_column = 1;
    int m_row = 1;
    float m_zoom = 1.0f;

    std::set<uint32_t> m_selections;
};

}  // namespace cave
