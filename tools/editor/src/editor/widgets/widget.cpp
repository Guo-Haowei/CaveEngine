#include "widget.h"

#include "engine/assets/image_asset.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/scene.h"

#include "editor/editor_window.h"

namespace cave {

void PushDisabled() {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
}

void PopDisabled() {
    ImGui::PopItemFlag();
    ImGui::PopStyleVar();
}

bool DrawDragInt(const char* p_label,
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

bool DrawDragFloat(const char* p_label,
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

static bool DrawVec3ControlImpl(int type,
                                const char* p_label,
                                Vector3f& p_out_vec3,
                                float p_reset_value,
                                float p_column_width) {
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

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, p_column_width);
    ImGui::Text("%s", p_label);
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    ImGui::PushFont(bold_font);

    auto draw_button = [&](int idx) {
        if (ImGui::Button(button_names[idx])) {
            p_out_vec3[idx] = p_reset_value;
            is_dirty = true;
        }
    };

    draw_button(0);

    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    is_dirty |= ImGui::DragFloat("##X", &p_out_vec3.x, speed, min, max, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
    ImGui::PushFont(bold_font);

    draw_button(1);

    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    is_dirty |= ImGui::DragFloat("##Y", &p_out_vec3.y, speed, min, max, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    ImGui::PushFont(bold_font);
    draw_button(2);
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    is_dirty |= ImGui::DragFloat("##Z", &p_out_vec3.z, speed, min, max, "%.2f");
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();

    ImGui::Columns(1);

    ImGui::PopID();
    return is_dirty;
}

bool DrawCheckBoxBitflag(const char* p_title, uint32_t& p_flags, const uint32_t p_bit) {
    bool enabled = (p_flags & p_bit);
    if (ImGui::Checkbox(p_title, &enabled)) {
        enabled ? (p_flags |= p_bit) : (p_flags &= ~p_bit);
        return true;
    }
    return false;
}

bool DrawVec3Control(const char* p_label,
                     Vector3f& p_out_vec3,
                     float p_reset_value,
                     float p_column_width) {
    return DrawVec3ControlImpl(TYPE_TRANSFORM, p_label, p_out_vec3, p_reset_value, p_column_width);
}

bool DrawColorControl(const char* p_label,
                      Vector3f& p_out_vec3,
                      float p_reset_value,
                      float p_column_width) {
    return DrawVec3ControlImpl(TYPE_COLOR, p_label, p_out_vec3, p_reset_value, p_column_width);
}

bool DrawInputText(const char* p_label,
                   std::string& p_string,
                   float p_column_width) {

    if (p_label) {
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, p_column_width);
        ImGui::Text("%s", p_label);
        ImGui::NextColumn();
    }

    char buffer[256];
    strncpy(buffer, p_string.c_str(), sizeof(buffer));
    auto tag = std::format("##{}", p_label ? p_label : "dummy");
    bool dirty = ImGui::InputText(tag.c_str(),
                                  buffer,
                                  sizeof(buffer),
                                  ImGuiInputTextFlags_EnterReturnsTrue);

    if (dirty) {
        p_string = buffer;
    }

    ImGui::Columns(1);
    return dirty;
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

bool ToggleButton(const char* p_str_id, bool* p_value) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.55f;
    float radius = height * 0.50f;

    bool toggled = false;

    ImGui::InvisibleButton(p_str_id, ImVec2(width, height));
    if (ImGui::IsItemClicked()) {
        *p_value = !*p_value;
        toggled = true;
    }

    float t = *p_value ? 1.0f : 0.0f;

    ImGuiContext& g = *GImGui;
    float ANIM_SPEED = 0.08f;
    if (g.LastActiveId == g.CurrentWindow->GetID(p_str_id))  // && g.LastActiveIdTimer < ANIM_SPEED)
    {
        float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        t = *p_value ? (t_anim) : (1.0f - t_anim);
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

bool DragDropTarget(AssetType p_mask,
                    const DragDropFunc& p_callback) {

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(ASSET_DRAG_DROP_PAYLOAD)) {
            const char* path = reinterpret_cast<const char*>(payload->Data);
            auto handle = AssetRegistry::GetSingleton().FindByPath(path, p_mask);
            if (handle.is_some()) {
                p_callback(handle.unwrap_unchecked());
            }
        }
        ImGui::EndDragDropTarget();
        return true;
    }

    return false;
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

void ShowAssetToolTip(const Guid& p_guid) {
    if (auto res = AssetRegistry::GetSingleton().FindByGuid(p_guid); res.is_some()) {
        AssetHandle handle = std::move(res.unwrap_unchecked());
        if (auto asset = handle.Get(); asset) {
            ShowAssetToolTip(*handle.GetMeta(), asset);
        }
    }
}

void ShowAssetToolTip(const AssetMetaData& p_meta, const IAsset* p_asset) {
    if (ImGui::BeginTooltip()) {
        ImGui::Text("%s", p_meta.path.c_str());
        ImGui::Text("type: %s", ToString(p_meta.type));

        if (p_asset) {
            if (p_asset->type == AssetType::Image) {
                auto texture = reinterpret_cast<const ImageAsset&>(*p_asset);
                const int w = texture.width;
                const int h = texture.height;
                ImGui::Text("Dimension: %d x %d", w, h);

                if (texture.gpu_texture) {
                    float adjusted_w = glm::min(256.f, static_cast<float>(w));
                    float adjusted_h = adjusted_w / w * h;
                    ImGui::Image(texture.gpu_texture->GetHandle(), ImVec2(adjusted_w, adjusted_h));
                }
            }
        }

        ImGui::EndTooltip();
    }
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
