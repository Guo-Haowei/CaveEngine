#include "inputs.h"

#include <imgui/imgui_internal.h>

namespace cave::ui {

using namespace ::cave::math;

bool CheckBox(const char* name,
              bool& value,
              float column_width) {
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, column_width);
    ImGui::Text("%s", name);
    ImGui::NextColumn();

    auto string_id = std::format("##{}", name);
    const bool dirty = ImGui::Checkbox(string_id.c_str(), &value);

    ImGui::Columns(1);
    return dirty;
}

bool TextBox(const char* label,
             char* buf_ptr,
             uint32_t buf_size,
             bool enter_returns_true,
             float column_width) {
    auto tag = std::format("##{}", label ? label : "dummy");
    auto columns_id = std::format("##{}_columns", label ? label : "dummy");

    if (label) {
        ImGui::Columns(2, columns_id.c_str());
        ImGui::SetColumnWidth(0, column_width);

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);

        // ImGui::Text("%s", label);
        ImGui::NextColumn();
    }

    int flags = enter_returns_true
                    ? ImGuiInputTextFlags_EnterReturnsTrue
                    : 0;

    bool dirty = ImGui::InputText(tag.c_str(),
                                  buf_ptr,
                                  buf_size,
                                  flags);
    if (label) {
        ImGui::Columns(1);
    }
    return dirty;
}

bool TextBox(const char* label,
             std::string& str,
             bool enter_returns_true,
             float column_width) {
    char buf[256]{};
    StringUtils::strcpy(buf, str.c_str());
    const bool dirty = TextBox(label,
                               buf,
                               sizeof(buf),
                               enter_returns_true,
                               column_width);
    if (dirty) {
        str = buf;
    }
    return dirty;
}

bool InputInt(const char* label,
              int& out_value,
              float column_width) {
    auto columns_id = std::format("##{}_columns", label);
    ImGui::Columns(2, columns_id.c_str());
    ImGui::SetColumnWidth(0, column_width);
    ImGui::Text("%s", label);
    ImGui::NextColumn();
    auto tag = std::format("##{}", label);
    bool is_dirty = ImGui::InputInt(tag.c_str(), &out_value);
    ImGui::Columns(1);
    return is_dirty;
}

bool InputFloat(const char* label,
                float& out_value,
                float column_width) {
    auto columns_id = std::format("##{}_columns", label);
    ImGui::Columns(2, columns_id.c_str());
    ImGui::SetColumnWidth(0, column_width);
    ImGui::Text("%s", label);
    ImGui::NextColumn();
    auto tag = std::format("##{}", label);
    bool is_dirty = ImGui::InputFloat(tag.c_str(), &out_value);
    ImGui::Columns(1);
    return is_dirty;
}

bool DragInt(const char* label,
             int& out_value,
             float speed,
             int min,
             int max,
             float column_width) {
    auto columns_id = std::format("##{}_columns", label);
    ImGui::Columns(2, columns_id.c_str());
    ImGui::SetColumnWidth(0, column_width);
    ImGui::Text("%s", label);
    ImGui::NextColumn();
    auto tag = std::format("##{}", label);
    bool is_dirty = ImGui::DragInt(tag.c_str(), &out_value, speed, min, max);
    ImGui::Columns(1);
    return is_dirty;
}

bool DragFloat(const char* label,
               float& out,
               float speed,
               float min,
               float max,
               float column_width) {
    auto columns_id = std::format("##{}_columns", label);
    ImGui::Columns(2, columns_id.c_str());
    ImGui::SetColumnWidth(0, column_width);
    ImGui::Text("%s", label);
    ImGui::NextColumn();
    auto tag = std::format("##{}", label);
    bool is_dirty = ImGui::DragFloat(tag.c_str(), &out, speed, min, max);
    ImGui::Columns(1);
    return is_dirty;
}

enum {
    TYPE_TRANSFORM,
    TYPE_COLOR,
};

template<int N>
static bool Float3Impl(int type,
                       const char* label,
                       float* data,
                       float reset_value,
                       float column_width) {
    static_assert(N >= 1 && N <= 3);
    bool is_dirty = false;

    ImGuiIO& io = ImGui::GetIO();
    auto bold_font = io.Fonts->Fonts[0];

    const char* button_names[3];
    float speed = 0.1f;
    float min = 0.0f;
    float max = 0.0f;
    if (type == TYPE_COLOR) {
        button_names[0] = "R";
        button_names[1] = "G";
        button_names[2] = "B";
        speed = 0.01f;
        min = 0.0f;
        max = 1.0f;
    } else {
        button_names[0] = "X";
        button_names[1] = "Y";
        button_names[2] = "Z";
    }

    ImGui::PushID(label);

    auto draw_button = [&](int idx) {
        if (ImGui::Button(button_names[idx])) {
            data[idx] = reset_value;
            is_dirty = true;
        }
    };

    auto columns_id = std::format("##{}_columns", label);
    ImGui::Columns(2, columns_id.c_str());
    ImGui::SetColumnWidth(0, column_width);
    ImGui::Text("%s", label);
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

    // x component
    if constexpr (N >= 1) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(bold_font);
        draw_button(0);
        ImGui::PopFont();
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        is_dirty |= ImGui::DragFloat("##X", &data[0], speed, min, max, "%.2f");
        ImGui::PopItemWidth();
    }

    // y component
    if constexpr (N >= 2) {
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(bold_font);
        draw_button(1);
        ImGui::PopFont();
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        is_dirty |= ImGui::DragFloat("##Y", &data[1], speed, min, max, "%.2f");
        ImGui::PopItemWidth();
    }

    // z component
    if constexpr (N >= 3) {
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushFont(bold_font);
        draw_button(2);
        ImGui::PopFont();
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        is_dirty |= ImGui::DragFloat("##Z", &data[2], speed, min, max, "%.2f");
        ImGui::PopItemWidth();
    }

    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();
    return is_dirty;
}
bool Float2(const char* label,
            Vec2f& out,
            float reset_value,
            float column_width) {
    return Float3Impl<2>(TYPE_TRANSFORM, label, &out.x, reset_value, column_width);
}

bool Float3(const char* label,
            Vec3f& out_vec3,
            float reset_value,
            float column_width) {
    return Float3Impl<3>(TYPE_TRANSFORM, label, &out_vec3.x, reset_value, column_width);
}

bool ColorPicker3(const char* label,
                  Vec3f& out,
                  float column_width) {
    auto columns_id = std::format("##{}_columns", label);
    ImGui::Columns(2, columns_id.c_str());
    ImGui::SetColumnWidth(0, column_width);
    ImGui::Text("%s", label);
    ImGui::NextColumn();
    const bool dirty = ImGui::ColorPicker3(label, &out.r);
    ImGui::Columns(1);
    ImGui::Dummy(ImVec2(8, 8));
    return dirty;
}

bool ColorPicker4(const char* label,
                  Vec4f& out,
                  float column_width) {
    auto columns_id = std::format("##{}_columns", label);
    ImGui::Columns(2, columns_id.c_str());
    ImGui::SetColumnWidth(0, column_width);
    ImGui::Text("%s", label);
    ImGui::NextColumn();
    const bool dirty = ImGui::ColorPicker4(label, &out.r);
    ImGui::Columns(1);
    ImGui::Dummy(ImVec2(8, 8));
    return dirty;
}

bool ToggleButton(const char* str_id, bool& value) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.55f;
    float radius = height * 0.50f;

    bool toggled = false;

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if (ImGui::IsItemClicked()) {
        value = !value;
        toggled = true;
    }

    float t = value ? 1.0f : 0.0f;

    ImGuiContext& g = *GImGui;
    float ANIM_SPEED = 0.08f;
    if (g.LastActiveId == g.CurrentWindow->GetID(str_id)) {
        float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        t = value ? (t_anim) : (1.0f - t_anim);
    }

    ImU32 col_bg;
    if (ImGui::IsItemHovered()) {
        col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), ImVec4(0.64f, 0.83f, 0.34f, 1.0f), t));
    } else {
        col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.56f, 0.83f, 0.26f, 1.0f), t));
    }

    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));

    return toggled;
}

bool DrawBitMask32(const char* str_id, uint32_t& value) {
    bool changed = false;

    ImGui::PushID(str_id);

    ImGui::TextUnformatted(str_id);

    constexpr int kColumns = 8;
    constexpr int kRows = 2;

    const ImVec2 button_size(32.0f, 24.0f);

    const ImVec4 selected_color = ImVec4(0.30f, 0.48f, 0.66f, 1.0f);
    const ImVec4 selected_hover_color = ImVec4(0.36f, 0.56f, 0.76f, 1.0f);
    const ImVec4 selected_active_color = ImVec4(0.24f, 0.42f, 0.60f, 1.0f);

    const ImVec4 unselected_color = ImVec4(0.15f, 0.22f, 0.30f, 1.0f);
    const ImVec4 unselected_hover_color = ImVec4(0.20f, 0.30f, 0.40f, 1.0f);
    const ImVec4 unselected_active_color = ImVec4(0.24f, 0.36f, 0.48f, 1.0f);

    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kColumns; ++col) {
            const int index = row * kColumns + col;
            const uint32_t bit = 1u << index;
            const bool enabled = (value & bit) != 0;

            ImGui::PushID(index);

            char text[8];
            std::snprintf(text, sizeof(text), "%d", index + 1);

            if (enabled) {
                ImGui::PushStyleColor(ImGuiCol_Button, selected_color);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, selected_hover_color);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, selected_active_color);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, unselected_color);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, unselected_hover_color);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, unselected_active_color);
            }

            if (ImGui::Button(text, button_size)) {
                value ^= bit;
                changed = true;
            }

            ImGui::PopStyleColor(3);

            ImGui::PopID();

            if (col + 1 < kColumns) {
                ImGui::SameLine(0.0f, 2.0f);
            }
        }
    }

    ImGui::PopID();

    return changed;
}

}  // namespace cave::ui
