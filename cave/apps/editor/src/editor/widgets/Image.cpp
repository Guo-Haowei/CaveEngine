#include "Image.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >
#include <imgui/imgui.h>

#include "engine/private/runtime/assets/ImageAsset.h"

namespace cave::ui {

void OkIcon() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.502f, 0.878f, 0.502f, 1.0f));
    ImGui::Text(ICON_FA_CIRCLE_CHECK);
    ImGui::PopStyleColor();
}

void WarningIcon() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.75f, 0.0f, 1.0f));
    ImGui::Text(ICON_FA_TRIANGLE_EXCLAMATION);
    ImGui::PopStyleColor();
}

void ErrorIcon() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
    ImGui::Text(ICON_FA_CIRCLE_EXCLAMATION);
    ImGui::PopStyleColor();
}

void CenteredImage(const ImageAsset* p_image,
                   const math::Vector2f& p_background_region,
                   uint64_t p_background) {

    ImVec2 background_region(p_background_region.x, p_background_region.y);
    ImVec2 image_region = background_region;
    uint64_t texture_handle = 0;
    if (p_image) {
        image_region.x = static_cast<float>(p_image->width);
        image_region.y = static_cast<float>(p_image->height);
        if (p_image->gpu_texture) {
            texture_handle = p_image->gpu_texture->GetHandle();
        }
    }

    ImGui::Dummy(ImVec2(8, 8));

    ImGui::BeginChild("CenteredImageRegion", background_region, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

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

    ImGui::Dummy(background_region);

    ImGui::EndChild();
}

auto AssetCard(uint64_t p_texture_id,
               const char* p_name,
               const math::Vector2f& p_image_size) -> std::tuple<bool, bool> {

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();

    const float rounding = 6.0f;
    const float padding = 6.0f;
    const float spacing = 4.0f;
    const float shadow_offset = 5.0f;

    // Estimate text height: 2 lines + padding
    float text_height = ImGui::GetFontSize() * 2 + spacing * 2;
    ImVec2 card_size = ImVec2(p_image_size.x + padding * 2,
                              p_image_size.y + text_height + 8);

    // Shadow behind card
    draw->AddRectFilled(pos + ImVec2(shadow_offset, shadow_offset),
                        pos + card_size + ImVec2(shadow_offset, shadow_offset),
                        IM_COL32(10, 10, 10, 160),
                        rounding);

    // Card background (lighter than ImGui window)
    ImU32 card_bg = IM_COL32(40, 40, 40, 255);
    ImGui::PushStyleColor(ImGuiCol_Button, card_bg);  // just for convention
    draw->AddRectFilled(pos, pos + card_size, card_bg, rounding);
    ImGui::PopStyleColor();

    ImGui::InvisibleButton(p_name, card_size);
    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();

    // Image (square)
    ImVec2 image_min = pos + ImVec2(padding, padding);
    ImVec2 image_max = image_min + ImVec2(p_image_size.x, p_image_size.y);

    draw->AddImage(p_texture_id, image_min, image_max);

    // Text
    ImVec2 textStart = image_min + ImVec2(0, p_image_size.y + spacing);
    draw->AddText(textStart, IM_COL32(180, 180, 180, 220), p_name);

    if (hovered) {
        draw->AddRect(pos, pos + card_size, IM_COL32(255, 255, 255, 100), rounding, 0, 1.5f);
    }

    return { hovered, clicked };
}

}  // namespace cave::ui
