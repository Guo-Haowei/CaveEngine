#include "log_panel.h"

#include "engine/debugger/profiler.h"
#include "engine/math/color.h"

#include "editor/widgets/image.h"

namespace cave {

static void DrawLog(const LogEvent& p_log) {
    switch (p_log.level) {
        case LOG_LEVEL_WARN:
            ui::WarningIcon();
            break;
        case LOG_LEVEL_ERROR:
        case LOG_LEVEL_FATAL:
            ui::ErrorIcon();
            break;
        default:
            ui::OkIcon();
            break;
    }

    Color color = Color::Hex(p_log.level == LOG_LEVEL_VERBOSE ? ColorCode::COLOR_SILVER : ColorCode::COLOR_WHITE);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.r, color.g, color.b, 1.0f));
    ImGui::SameLine();
    ImGui::Text("  %s", p_log.message.c_str());
    ImGui::PopStyleColor();
}

void LogPanel::UpdateInternal() {
    CAVE_PROFILE_EVENT();

    ImGui::Separator();

    // reserve enough left-over height for 1 separator + 1 input text
    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));  // Tighten spacing

    ImVec2 window_size = ImGui::GetWindowSize();

    constexpr ImU32 colors[] = {
        IM_COL32(57, 57, 57, 255),  // light
        IM_COL32(41, 42, 44, 255),  // dark
    };

    constexpr float padding = 3;

    int color_index = 0;

    const std::vector<LogEvent>* logs = &CompositeLogger::GetSingleton().GetAllLogs();
    switch (m_filter) {
        case cave::LOG_LEVEL_WARN:
            logs = &CompositeLogger::GetSingleton().GetWarningLogs();
            break;
        case cave::LOG_LEVEL_ERROR:
            logs = &CompositeLogger::GetSingleton().GetErrorLogs();
            break;
        default:
            break;
    }

    for (const LogEvent& log : (*logs)) {
        if (log.level & m_filter) {
            ImVec2 text_pos = ImGui::GetCursorScreenPos();
            ImVec2 text_size = ImGui::CalcTextSize(log.message.c_str());
            text_size.x = std::max(text_size.x, window_size.x);
            text_size.y += padding * 3;

            ImGui::GetWindowDrawList()->AddRectFilled(
                text_pos,
                ImVec2(text_pos.x + text_size.x, text_pos.y + text_size.y),
                colors[color_index]);

            color_index ^= 1;

            ImGui::Dummy(ImVec2(padding, padding));
            DrawLog(log);
            ImGui::Dummy(ImVec2(padding, padding));
        }
    }

    if (m_scroll_to_bottom || (m_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())) {
        ImGui::SetScrollHereY(1.0f);
    }
    m_scroll_to_bottom = false;

    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::Separator();

    if (ImGui::SmallButton("All")) {
        m_filter = LOG_LEVEL_ALL;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("No Verbose")) {
        m_filter = LOG_LEVEL_ALL & (~LOG_LEVEL_VERBOSE);
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Warning")) {
        m_filter = LOG_LEVEL_WARN;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Error")) {
        m_filter = LOG_LEVEL_ERROR;
    }
    ImGui::SameLine();

    ImGui::Separator();

    // Auto-focus on window apparition
    ImGui::SetItemDefaultFocus();
}

}  // namespace cave
