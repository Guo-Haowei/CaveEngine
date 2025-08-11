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

    static uint32_t Pack(uint16_t x, uint16_t y) {
        return x << 16 | y;
    }

    static std::pair<uint16_t, uint16_t> Unpack(uint32_t p_key) {
        return {
            static_cast<uint16_t>(p_key >> 16),
            static_cast<uint16_t>(p_key & 0xFFFF),
        };
    }

private:
    const SelectionMode m_mode;

    int m_column = 1;
    int m_row = 1;
    float m_zoom = 1.0f;

    std::set<uint32_t> m_selections;
};

}  // namespace cave
