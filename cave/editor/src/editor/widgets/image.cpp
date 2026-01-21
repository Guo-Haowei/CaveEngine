#include "image.h"

#include <imgui/imgui.h>

#include "engine/assets/image_asset.h"

namespace cave::ui {

void CenteredImage(const ImageAsset* p_image,
                   const Vector2f& p_background_region,
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

}  // namespace cave::ui
