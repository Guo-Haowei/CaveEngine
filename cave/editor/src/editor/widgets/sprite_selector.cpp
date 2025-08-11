#include "sprite_selector.h"

#include "engine/assets/image_asset.h"

#include "editor/widgets/widget.h"

namespace cave {

bool SpriteSelector::EditSprite(int* p_colomn, int* p_row) {
    bool dirty = false;
    if (ImGui::BeginTabBar("TileSetModes")) {
        if (ImGui::BeginTabItem("Setup")) {
            if (!p_colomn) {
                p_colomn = &m_column;
            }
            if (!p_row) {
                p_row = &m_row;
            }
            if (ImGui::InputInt("column", p_colomn)) {
                *p_colomn = std::max(*p_colomn, 1);
                dirty = true;
            }
            if (ImGui::InputInt("row", p_row)) {
                *p_row = std::max(*p_row, 1);
                dirty = true;
            }

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    m_column = *p_colomn;
    m_row = *p_row;
    return dirty;
}

std::vector<std::pair<uint16_t, uint16_t>> SpriteSelector::GetSelections() const {
    std::vector<std::pair<uint16_t, uint16_t>> selections;
    selections.reserve(m_selections.size());
    for (uint32_t key : m_selections) {
        selections.push_back(Unpack(key));
    }
    return selections;
}

void SpriteSelector::SelectSprite(const ImageAsset& p_image,
                                  const int* p_colomn,
                                  const int* p_row) {
    DrawDragFloat("scale", m_zoom, 0.01f, 0.1f, 5.0f);

    const float width = m_zoom * p_image.width;
    const float height = m_zoom * p_image.height;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 tile_size(width, height);

    ImGui::InvisibleButton("TileClickable", tile_size);  // enables interaction
    const bool hovered = ImGui::IsItemHovered();
    const bool left_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
    const bool right_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);
    const bool clicked = left_clicked || right_clicked;

    // Draw image
    draw_list->AddImage(
        p_image.gpu_texture ? p_image.gpu_texture->GetHandle() : 0,
        cursor,
        cursor + tile_size,
        ImVec2(0, 0), ImVec2(1, 1),
        IM_COL32(255, 255, 255, 255));

    const int num_col = p_colomn ? *p_colomn : m_column;
    const int num_row = p_row ? *p_row : m_row;
    m_column = num_col;
    m_row = num_row;

    const float cell_w = width / num_col;
    const float cell_h = height / num_row;

    // Draw lines
    for (float dx = cell_w; dx < tile_size.x; dx += cell_w) {
        draw_list->AddLine(ImVec2(cursor.x + dx, cursor.y),
                           ImVec2(cursor.x + dx, cursor.y + tile_size.y),
                           IM_COL32(255, 255, 255, 255));
    }

    for (float dy = cell_h; dy < tile_size.y; dy += cell_h) {
        draw_list->AddLine(ImVec2(cursor.x, cursor.y + dy),
                           ImVec2(cursor.x + tile_size.x, cursor.y + dy),
                           IM_COL32(255, 255, 255, 255));
    }

    if (hovered && clicked) {
        ImVec2 mouse_pos = ImGui::GetMousePos();

        int local_x = (int)((mouse_pos.x - cursor.x) / cell_w);
        int local_y = (int)((mouse_pos.y - cursor.y) / cell_h);

        // Clamp to valid range
        if (local_x >= 0 && local_x < num_col && local_y >= 0 && local_y < num_row) {
            uint32_t key = Pack(static_cast<uint16_t>(local_x), static_cast<uint16_t>(local_y));
            switch (m_mode) {
                case SelectionMode::Single: {
                    if (left_clicked) {
                        if (!m_selections.empty()) {
                            m_selections.clear();
                        }
                        m_selections.insert(key);
                    }
                } break;
                case SelectionMode::Multi: {
                    if (left_clicked) {
                        m_selections.insert(key);
                    } else if (right_clicked) {
                        m_selections.erase(key);
                    }
                } break;
            }
        }
    }

    for (uint32_t key : m_selections) {
        const auto [x, y] = Unpack(key);

        ImVec2 p_min = ImVec2(cursor.x + x * cell_w, cursor.y + y * cell_h);
        ImVec2 p_max = ImVec2(p_min.x + cell_w, p_min.y + cell_h);

        draw_list->AddRectFilled(p_min, p_max, IM_COL32(0, 255, 0, 100));  // green transparent overlay
        draw_list->AddRect(p_min, p_max, IM_COL32(0, 255, 0, 255));        // solid border
    }
}

}  // namespace cave
