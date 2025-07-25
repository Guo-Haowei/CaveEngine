#include "tool_bar.h"

#include <imgui/imgui_internal.h>

namespace cave {

static void DrawToolBarButton(const ToolBarButtonDesc& desc) {
    const bool enabled = desc.is_enabled_func ? desc.is_enabled_func() : true;

    if (!enabled) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    }

    if (ImGui::Button(desc.display) && enabled) {
        desc.execute_func();
    }

    if (!enabled) {
        ImGui::PopStyleVar();
    }

    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("%s", desc.tooltip);
        ImGui::EndTooltip();
    }
}

void DrawToolBar(const std::vector<const ToolBarButtonDesc*>& p_button_descs) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    auto& colors = ImGui::GetStyle().Colors;
    const auto& button_hovered = colors[ImGuiCol_ButtonHovered];
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(button_hovered.x, button_hovered.y, button_hovered.z, 0.5f));
    const auto& button_active = colors[ImGuiCol_ButtonActive];
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(button_active.x, button_active.y, button_active.z, 0.5f));

    for (size_t i = 0; i < p_button_descs.size(); ++i) {
        const ToolBarButtonDesc* desc = p_button_descs[i];
        if (i != 0) ImGui::SameLine();
        DrawToolBarButton(*desc);
    }

    // ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

}  // namespace cave
