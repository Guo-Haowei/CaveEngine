#include "inputs.h"

#include <imgui/imgui_internal.h>

namespace cave::ui {

bool CheckBox(const char* p_name,
              bool& p_val,
              float p_column_width) {
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, p_column_width);
    ImGui::Text("%s", p_name);
    ImGui::NextColumn();

    auto string_id = std::format("##{}", p_name);
    const bool dirty = ImGui::Checkbox(string_id.c_str(), &p_val);

    ImGui::Columns(1);
    return dirty;
}

bool TextBox(const char* p_label,
             char* p_buf_ptr,
             uint32_t p_buf_size,
             float p_text_width,
             float p_text_box_width,
             bool p_enter_returns_true) {
    if (p_label) {
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, p_text_width);
        if (p_text_box_width > 0) {
            ImGui::SetColumnWidth(1, p_text_box_width);
        }
        ImGui::Text("%s", p_label);
        ImGui::NextColumn();
    }

    int flags = 0;
    if (p_enter_returns_true) {
        flags |= ImGuiInputTextFlags_EnterReturnsTrue;
    }

    auto tag = std::format("##{}", p_label ? p_label : "dummy");
    bool dirty = ImGui::InputText(tag.c_str(),
                                  p_buf_ptr,
                                  p_buf_size,
                                  flags);

    ImGui::Columns(1);
    return dirty;
}
bool InputInt(const char* p_label,
              int& p_out,
              float p_column_width) {
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, p_column_width);
    ImGui::Text("%s", p_label);
    ImGui::NextColumn();
    auto tag = std::format("##{}", p_label);
    bool is_dirty = ImGui::InputInt(tag.c_str(), &p_out);
    ImGui::Columns(1);
    return is_dirty;
}

bool InputFloat(const char* p_label,
                float& p_out,
                float p_column_width) {
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, p_column_width);
    ImGui::Text("%s", p_label);
    ImGui::NextColumn();
    auto tag = std::format("##{}", p_label);
    bool is_dirty = ImGui::InputFloat(tag.c_str(), &p_out);
    ImGui::Columns(1);
    return is_dirty;
}

bool DragInt(const char* p_label,
             int& p_out,
             float p_speed,
             int p_min,
             int p_max,
             float p_column_width) {
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, p_column_width);
    ImGui::Text("%s", p_label);
    ImGui::NextColumn();
    auto tag = std::format("##{}", p_label);
    bool is_dirty = ImGui::DragInt(tag.c_str(), &p_out, p_speed, p_min, p_max);
    ImGui::Columns(1);
    return is_dirty;
}

bool DragFloat(const char* p_label,
               float& p_out,
               float p_speed,
               float p_min,
               float p_max,
               float p_column_width) {
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, p_column_width);
    ImGui::Text("%s", p_label);
    ImGui::NextColumn();
    auto tag = std::format("##{}", p_label);
    bool is_dirty = ImGui::DragFloat(tag.c_str(), &p_out, p_speed, p_min, p_max);
    ImGui::Columns(1);
    return is_dirty;
}

enum {
    TYPE_TRANSFORM,
    TYPE_COLOR,
};

template<int N>
static bool Float3Impl(int type,
                       const char* p_label,
                       float* p_data,
                       float p_reset_value,
                       float p_column_width) {
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

    ImGui::PushID(p_label);

    auto draw_button = [&](int idx) {
        if (ImGui::Button(button_names[idx])) {
            p_data[idx] = p_reset_value;
            is_dirty = true;
        }
    };

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, p_column_width);
    ImGui::Text("%s", p_label);
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
        is_dirty |= ImGui::DragFloat("##X", &p_data[0], speed, min, max, "%.2f");
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
        is_dirty |= ImGui::DragFloat("##Y", &p_data[1], speed, min, max, "%.2f");
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
        is_dirty |= ImGui::DragFloat("##Z", &p_data[2], speed, min, max, "%.2f");
        ImGui::PopItemWidth();
    }

    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();
    return is_dirty;
}
bool Float2(const char* p_label,
            math::Vector2f& p_out,
            float p_reset_value,
            float p_column_width) {
    return Float3Impl<2>(TYPE_TRANSFORM, p_label, &p_out.x, p_reset_value, p_column_width);
}

bool Float3(const char* p_label,
            math::Vector3f& p_out_vec3,
            float p_reset_value,
            float p_column_width) {
    return Float3Impl<3>(TYPE_TRANSFORM, p_label, &p_out_vec3.x, p_reset_value, p_column_width);
}

bool ColorPicker3(const char* p_label,
                  float* p_out,
                  float p_column_width) {
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, p_column_width);
    ImGui::Text("%s", p_label);
    ImGui::NextColumn();
    const bool dirty = ImGui::ColorPicker3(p_label, p_out);
    ImGui::Columns(1);
    return dirty;
}

bool ColorPicker4(const char* p_label,
                  float* p_out,
                  float p_column_width) {
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, p_column_width);
    ImGui::Text("%s", p_label);
    ImGui::NextColumn();
    const bool dirty = ImGui::ColorPicker4(p_label, p_out);
    ImGui::Columns(1);
    ImGui::Dummy(ImVec2(8, 8));
    return dirty;
}
bool ToggleButton(const char* p_str_id, bool& p_value) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.55f;
    float radius = height * 0.50f;

    bool toggled = false;

    ImGui::InvisibleButton(p_str_id, ImVec2(width, height));
    if (ImGui::IsItemClicked()) {
        p_value = !p_value;
        toggled = true;
    }

    float t = p_value ? 1.0f : 0.0f;

    ImGuiContext& g = *GImGui;
    float ANIM_SPEED = 0.08f;
    if (g.LastActiveId == g.CurrentWindow->GetID(p_str_id)) {
        float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        t = p_value ? (t_anim) : (1.0f - t_anim);
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

}  // namespace cave::ui
