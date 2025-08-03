#include "log_panel.h"

#include "engine/core/debugger/profiler.h"
#include "engine/core/io/logger.h"
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

void LogPanel::UpdateInternal() {
    CAVE_PROFILE_EVENT();

    ImGui::Separator();

    // reserve enough left-over height for 1 separator + 1 input text
    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));  // Tighten spacing

    auto& logger = CompositeLogger::GetSingleton();
    std::vector<cave::CompositeLogger::Log> logs;
    logger.RetrieveLog(logs);

    m_warning_count = m_error_count = 0;

    for (const auto& log : logs) {
        if (log.level & LogLevel::LOG_LEVEL_ERROR) {
            ++m_error_count;
        } else if (log.level & LogLevel::LOG_LEVEL_WARN) {
            ++m_warning_count;
        }

        if (log.level & m_filter) {
            ImGui::PushStyleColor(ImGuiCol_Text, GetLogLevelColor(log.level));
            ImGui::TextUnformatted(log.buffer);
            ImGui::PopStyleColor();
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
