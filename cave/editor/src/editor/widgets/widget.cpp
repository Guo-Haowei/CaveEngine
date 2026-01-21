#include "widget.h"

#include "engine/assets/image_asset.h"
#include "engine/scene/scene.h"

namespace cave {

void PushDisabled() {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
}

void PopDisabled() {
    ImGui::PopItemFlag();
    ImGui::PopStyleVar();
}

bool DrawCheckBoxBitflag(const char* p_title, uint32_t& p_flags, const uint32_t p_bit) {
    bool enabled = (p_flags & p_bit);
    if (ImGui::Checkbox(p_title, &enabled)) {
        enabled ? (p_flags |= p_bit) : (p_flags &= ~p_bit);
        return true;
    }
    return false;
}

bool DrawColorPicker3(const char* p_label,
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

bool DrawColorPicker4(const char* p_label,
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

void CenteredImage(const ImageAsset* p_image,
                   const ImVec2& p_background_region,
                   uint64_t p_background) {

    ImVec2 image_region = p_background_region;
    uint64_t texture_handle = 0;
    if (p_image) {
        image_region.x = static_cast<float>(p_image->width);
        image_region.y = static_cast<float>(p_image->height);
        if (p_image->gpu_texture) {
            texture_handle = p_image->gpu_texture->GetHandle();
        }
    }

    ImGui::Dummy(ImVec2(8, 8));

    ImGui::BeginChild("CenteredImageRegion", p_background_region, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::GetWindowDrawList()->AddImage(
        p_background,
        pos,
        ImVec2(pos.x + p_background_region.x, pos.y + p_background_region.y));

    if (texture_handle) {
        const float ratio = image_region.x / image_region.y;
        if (image_region.x > image_region.y) {
            image_region.x = p_background_region.x;
            image_region.y = image_region.x / ratio;
        } else {
            image_region.y = p_background_region.y;
            image_region.x = image_region.y * ratio;
        }

        ImVec2 offset = {
            (p_background_region.x - image_region.x) * 0.5f,
            (p_background_region.y - image_region.y) * 0.5f
        };

        pos.x += offset.x;
        pos.y += offset.y;
        ImGui::GetWindowDrawList()->AddImage(
            texture_handle,
            pos,
            ImVec2(pos.x + image_region.x, pos.y + image_region.y));
    }

    ImGui::Dummy(p_background_region);

    ImGui::EndChild();
}

void DrawContents(float p_full_width, const std::vector<AssetChildPanel>& p_descs) {
    const int size = static_cast<int>(p_descs.size());
    float width_so_far = 0.0f;
    for (int i = 0; i < size; ++i) {
        const auto& desc = p_descs[i];
        const bool is_last = i + 1 == size;

        const float width = is_last ? p_full_width - width_so_far : desc.width;
        width_so_far += width;

        ImGui::BeginChild(desc.name, ImVec2(width, 0), true);
        desc.func();
        ImGui::EndChild();

        if (!is_last) {
            ImGui::SameLine();
        }
    }
}

}  // namespace cave
