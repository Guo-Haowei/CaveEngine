#include "LogPanel.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/math/Color.h"
#include "engine/private/core/diagnostics/console/Console.h"
#include "engine/private/core/string/StringUtils.h"

#include "editor/EditorState.h"
#include "editor/widgets/Image.h"

namespace cave {

std::string_view LogPanel::AutoCompletion::Current() const {
    DEV_ASSERT(!m_cmds.empty());
    return m_cmds[m_index];
}

std::string_view LogPanel::AutoCompletion::Next() {
    const size_t size = m_cmds.size();
    DEV_ASSERT(size);
    m_index = (m_index + 1) % size;
    return m_cmds[m_index];
}

LogPanel::LogPanel(EditorState& p_editor)
    : EditorWindow(p_editor)
    , m_console(p_editor.GetApp().Console()) {
}

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
    if (p_log.repeat > 1) {
        ImGui::SameLine();
        ImGui::Text(" [ x%u ]", p_log.repeat);
    }
    ImGui::PopStyleColor();
}

void LogPanel::DrawFilter() {
    // @TODO: make filter a combo box
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
}

int LogPanel::InputCallback(ImGuiInputTextCallbackData* p_data) {
    LogPanel* self = reinterpret_cast<LogPanel*>(p_data->UserData);
    DEV_ASSERT(self);
    Console& console = self->m_console;

    const char* buf = p_data->Buf;
    const int text_length = p_data->BufTextLen;
    std::string_view line{ buf, buf + text_length };

    std::string_view candidate;
    switch (p_data->EventFlag) {
        case ImGuiInputTextFlags_CallbackCompletion: {
            if (line.empty()) break;

            if (!self->m_ac.Empty()) {
                candidate = self->m_ac.Next();
            } else {
                std::vector<std::string_view> cmds;
                console.FindByPrefix(line, cmds);
                if (!cmds.empty()) {
                    self->m_ac.Set(std::move(cmds));
                    candidate = self->m_ac.Current();
                }
            }
        } break;
        case ImGuiInputTextFlags_CallbackHistory: {
            if (p_data->EventKey == ImGuiKey_UpArrow) {
                Option<std::string_view> cmd = console.Prev();
                if (cmd.is_none()) break;
                candidate = cmd.unwrap_unchecked();
            } else if (p_data->EventKey == ImGuiKey_DownArrow) {
                Option<std::string_view> cmd = console.Next();
                if (cmd.is_none()) break;
                candidate = cmd.unwrap_unchecked();
            }
        } break;
        case ImGuiInputTextFlags_CallbackEdit: {
            // If user typed/edited, invalidate suggestions.
            self->m_ac.Clear();
        } break;
        default:
            break;
    }
    if (!candidate.empty()) {
        // @TODO: don't need to delete the previous chars,
        // also save the draft
        StringUtils::Strcpy(self->m_cmd_buffer, candidate);
        p_data->DeleteChars(0, text_length);
        p_data->InsertChars(0, self->m_cmd_buffer);
    }
    return 0;
}

void LogPanel::DrawConsole() {
    float spacing = 10.0f;
    ImGui::SetNextItemWidth(70.0f);
    ImGui::Text(">: cmd");
    ImGui::SameLine(0.0f, spacing);

    ImGui::SetNextItemWidth(-1.0f);

    const int flags = ImGuiInputTextFlags_CallbackEdit |
                      ImGuiInputTextFlags_CallbackCompletion |
                      ImGuiInputTextFlags_CallbackHistory |
                      ImGuiInputTextFlags_CallbackCharFilter |
                      ImGuiInputTextFlags_CallbackResize |
                      ImGuiInputTextFlags_EnterReturnsTrue;

    const bool submit = ImGui::InputTextWithHint(
        "##ConsoleInput",
        "Enter Console Command",
        m_cmd_buffer,
        IM_ARRAYSIZE(m_cmd_buffer),
        flags,
        &InputCallback,
        this);

    if (submit) {
        m_console.SubmitLine(m_cmd_buffer);
        m_cmd_buffer[0] = '\0';

        // Keep keyboard focus on the input (so you can type multiple commands quickly)
        ImGui::SetKeyboardFocusHere(-1);
    }
}

void LogPanel::DrawLogHistroy() {
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
}

void LogPanel::DrawUIImpl() {
    CAVE_PROFILE_EVENT();

    DrawFilter();
    ImGui::Separator();
    DrawLogHistroy();
    ImGui::Separator();
    DrawConsole();

    // Auto-focus on window apparition
    ImGui::SetItemDefaultFocus();
}

}  // namespace cave
