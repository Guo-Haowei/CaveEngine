#include "layout.h"

#include <imgui/imgui.h>

namespace cave {

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

bool BeginFullscreenWindow(const char* p_name) {
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoDocking;

    return ImGui::Begin(p_name, nullptr, flags);
}

void EndFullscreenWindow() {
    ImGui::End();
    ImGui::PopStyleVar(3);
}

}  // namespace cave