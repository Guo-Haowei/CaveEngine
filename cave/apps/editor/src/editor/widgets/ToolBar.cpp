#include "ToolBar.h"

#include <imgui/imgui_internal.h>

namespace cave {

static void DrawToolBarButton(const ToolbarButtonDesc& desc) {
    const bool enabled = desc.is_enabled_func ? desc.is_enabled_func() : true;
    const bool selected = desc.is_selected_func ? desc.is_selected_func() : false;

    ImGui::PushID(desc.id);

    ImGui::BeginDisabled(!enabled);

    if (selected) {
        const ImVec4 active = ImVec4(0.35f, 0.3505f, 0.351f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, active);
    }

    if (ImGui::Button(desc.display) && desc.execute_func) {
        desc.execute_func();
    }

    if (selected) {
        ImGui::PopStyleColor();
    }

    ImGui::EndDisabled();

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
        desc.tooltip) {
        ImGui::SetTooltip("%s", desc.tooltip);
    }

    ImGui::PopID();
}

void DrawToolbar(std::span<const ToolbarButtonDesc*> button_descs) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    auto& colors = ImGui::GetStyle().Colors;
    const auto& button_hovered = colors[ImGuiCol_ButtonHovered];
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(button_hovered.x, button_hovered.y, button_hovered.z, 0.5f));
    const auto& button_active = colors[ImGuiCol_ButtonActive];
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(button_active.x, button_active.y, button_active.z, 0.5f));

    for (size_t i = 0; i < button_descs.size(); ++i) {
        const ToolbarButtonDesc* desc = button_descs[i];
        DrawToolBarButton(*desc);
        ImGui::SameLine();
    }

    ImGui::PopStyleColor(3);
}

void DrawToolbar(std::span<const ToolbarButtonDesc> button_descs) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    auto& colors = ImGui::GetStyle().Colors;
    const auto& button_hovered = colors[ImGuiCol_ButtonHovered];
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(button_hovered.x, button_hovered.y, button_hovered.z, 0.5f));
    const auto& button_active = colors[ImGuiCol_ButtonActive];
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(button_active.x, button_active.y, button_active.z, 0.5f));

    for (size_t i = 0; i < button_descs.size(); ++i) {
        const ToolbarButtonDesc& desc = button_descs[i];
        DrawToolBarButton(desc);
        ImGui::SameLine();
    }

    ImGui::PopStyleColor(3);
}

}  // namespace cave
