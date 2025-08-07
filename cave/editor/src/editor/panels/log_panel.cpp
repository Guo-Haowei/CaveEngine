#include "log_panel.h"

#include "engine/debugger/profiler.h"
#include "engine/math/color.h"

namespace cave {

static ImVec4 GetLogLevelColor(LogLevel level) {
    Color color = Color::Hex(ColorCode::COLOR_WHITE);
    switch (level) {
        case LOG_LEVEL_VERBOSE:
            color = Color::Hex(ColorCode::COLOR_SILVER);
            break;
        case LOG_LEVEL_OK:
            color = Color::Hex(ColorCode::COLOR_GREEN);
            break;
        case LOG_LEVEL_WARN:
            color = Color::Hex(ColorCode::COLOR_YELLOW);
            break;
        case LOG_LEVEL_ERROR:
        case LOG_LEVEL_FATAL:
            color = Color::Hex(ColorCode::COLOR_RED);
            break;
        default:
            break;
    }

    return ImVec4(color.r, color.g, color.b, 1.0f);
}

void LogPanel::RetrieveLogs() {
    auto& logger = CompositeLogger::GetSingleton();
    m_logs.clear();
    logger.RetrieveLog(m_logs);

    m_warning_count = 0;
    m_error_count = 0;

    for (const auto& log : m_logs) {
        if (log.level & LogLevel::LOG_LEVEL_ERROR) {
            ++m_error_count;
        } else if (log.level & LogLevel::LOG_LEVEL_WARN) {
            ++m_warning_count;
        }
    }
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

    constexpr float padding = 4;

    int color_index = 0;

    for (const auto& log : m_logs) {
        if (log.level & m_filter) {
            ImVec2 text_pos = ImGui::GetCursorScreenPos();
            ImVec2 text_size = ImGui::CalcTextSize(log.buffer);
            text_size.x = std::max(text_size.x, window_size.x);
            text_size.y += padding * 3;

            ImGui::GetWindowDrawList()->AddRectFilled(
                text_pos,
                ImVec2(text_pos.x + text_size.x, text_pos.y + text_size.y),
                colors[color_index]);

            color_index ^= 1;

            ImGui::Dummy(ImVec2(padding, padding));
            ImGui::PushStyleColor(ImGuiCol_Text, GetLogLevelColor(log.level));
            ImGui::Text(log.buffer);
            ImGui::PopStyleColor();
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
