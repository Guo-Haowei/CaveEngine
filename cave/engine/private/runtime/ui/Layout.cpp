#include "Layout.h"

#include <imgui/imgui_internal.h>

namespace cave::ui {

void DockSpace(const DockSpaceContext& p_context) {
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground;

    if (p_context.menu_bar_func) {
        flags |= ImGuiWindowFlags_MenuBar;
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin(p_context.str_id, nullptr, flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);

    if (p_context.menu_bar_func) {
        if (ImGui::BeginMainMenuBar()) {
            p_context.menu_bar_func();
        }
        ImGui::EndMainMenuBar();
    }

    if (p_context.side_bar_func) {
        if (ImGui::BeginViewportSideBar("StatusBar", viewport, ImGuiDir_Down, ImGui::GetFrameHeight(), ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar)) {
            if (ImGui::BeginMenuBar()) {
                p_context.side_bar_func();
                ImGui::EndMenuBar();
            }
            ImGui::End();
        }
    }
    ImGui::End();
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

}  // namespace cave::ui