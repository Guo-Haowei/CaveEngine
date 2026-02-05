#include "LogPanel.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/math/Color.h"
#include "engine/private/core/debugger/Profiler.h"
#include "engine/private/core/debugger/console/Console.h"

#include "editor/EditorState.h"
#include "editor/widgets/Image.h"

namespace cave {

LogPanel::LogPanel(EditorState& p_editor)
    : EditorWindow(p_editor)
    , m_console(p_editor.GetApp().Console()) {
    using namespace debug;

    CommandRegistry& reg = p_editor.GetApp().CommandRegistry();
    reg.Register({
        .name = "dump.pool",
        .fn = [](const CommandContext&, const CommandArgs&) {},
    });
    reg.Register({
        .name = "dump.asset",
        .fn = [](const CommandContext&, const CommandArgs&) {},
    });
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

#if 0
static int Stricmp(const char* a, const char* b) {
    for (;; a++, b++) {
        int d = std::tolower((unsigned char)*a) - std::tolower((unsigned char)*b);
        if (d != 0 || !*a || !*b) return d;
    }
}
static int Strnicmp(const char* a, const char* b, int n) {
    while (n-- > 0) {
        int d = std::tolower((unsigned char)*a) - std::tolower((unsigned char)*b);
        if (d != 0 || !*a || !*b) return d;
        a++;
        b++;
    }
    return 0;
}
#endif

int LogPanel::InputCallback(ImGuiInputTextCallbackData* p_data) {
    LogPanel* self = reinterpret_cast<LogPanel*>(p_data->UserData);
    DEV_ASSERT(self);
    debug::Console& console = self->m_console;
    unused(console);

    switch (p_data->EventFlag) {
        case ImGuiInputTextFlags_CallbackCompletion: {
            LOG("Auto complete");
        } break;
        default:
            break;
    }
#if 0
   switch (data->EventFlag) {
        case ImGuiInputTextFlags_CallbackCompletion: {
            // Find word start (simple: last token separated by space)
            const char* buf = data->Buf;
            int word_end = data->CursorPos;
            int word_start = word_end;
            while (word_start > 0 && buf[word_start - 1] != ' ' && buf[word_start - 1] != '\t')
                word_start--;

            const std::string current(buf + word_start, buf + word_end);
            if (current.empty()) break;

            // Find matches
            std::vector<const char*> candidates{ "dump" };
            // for (auto& c : self->commands)
            //     if (Strnicmp(c.c_str(), current.c_str(), (int)current.size()) == 0)
            //         candidates.push_back(c.c_str());

            if (candidates.empty()) {
                // no-op
            } else if (candidates.size() == 1) {
                // Single match: replace token with full command + trailing space
                data->DeleteChars(word_start, word_end - word_start);
                data->InsertChars(word_start, candidates[0]);
                data->InsertChars(data->CursorPos, " ");
            } else {
                // Multiple matches: common prefix extension
                int match_len = (int)current.size();
                for (;;) {
                    char ch = 0;
                    bool all_same = true;
                    for (int i = 0; i < (int)candidates.size(); i++) {
                        if ((int)std::strlen(candidates[i]) <= match_len) {
                            all_same = false;
                            break;
                        }
                        char cch = (char)std::tolower((unsigned char)candidates[i][match_len]);
                        if (i == 0)
                            ch = cch;
                        else if (ch != cch) {
                            all_same = false;
                            break;
                        }
                    }
                    if (!all_same) break;
                    match_len++;
                }

                if (match_len > (int)current.size()) {
                    data->DeleteChars(word_start, word_end - word_start);
                    data->InsertChars(word_start, candidates[0], candidates[0] + match_len);
                }

                // Optional: print candidates to your output log / console scrollback
                // for (auto* s : candidates) ConsoleOutput::Print(s);
            }
            break;
        }
        case ImGuiInputTextFlags_CallbackEdit: {
            const char* buf = data->Buf;
            int word_end = data->CursorPos;
            LOG("edit: {}, {}", buf, buf[word_end]);
        } break;
        case ImGuiInputTextFlags_CallbackHistory: {
            const int prev = self->historyPos;
            if (data->EventKey == ImGuiKey_UpArrow) {
                if (self->historyPos == -1)
                    self->historyPos = (int)self->history.size() - 1;
                else if (self->historyPos > 0)
                    self->historyPos--;
            } else if (data->EventKey == ImGuiKey_DownArrow) {
                if (self->historyPos != -1)
                    if (++self->historyPos >= (int)self->history.size())
                        self->historyPos = -1;
            }

            if (prev != self->historyPos) {
                const char* hist = (self->historyPos >= 0) ? self->history[self->historyPos].c_str() : "";
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, hist);
            }
            break;
        }
        default:
            break;
    }
#endif
    return 0;
}

void LogPanel::DrawConsole() {
    float spacing = 10.0f;
    int cmdModeIndex = 0;
    static const char* kModes[] = { "Cmd", "Render", "Scene", "Asset" };  // example
    ImGui::SetNextItemWidth(70.0f);
    if (ImGui::BeginCombo("##CmdMode", kModes[cmdModeIndex], ImGuiComboFlags_NoPreview)) {
        for (int i = 0; i < IM_ARRAYSIZE(kModes); i++) {
            const bool selected = (i == cmdModeIndex);
            if (ImGui::Selectable(kModes[i], selected))
                cmdModeIndex = i;
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine(0.0f, spacing);

    // Right: input field
    // Fill remaining width
    ImGui::SetNextItemWidth(-1.0f);

    static char input[128]{ 0 };

    const int flags = ImGuiInputTextFlags_CallbackEdit |
                      ImGuiInputTextFlags_CallbackCompletion |
                      ImGuiInputTextFlags_CallbackHistory |
                      ImGuiInputTextFlags_CallbackCharFilter |
                      ImGuiInputTextFlags_CallbackResize |
                      ImGuiInputTextFlags_EnterReturnsTrue;

    const bool submit = ImGui::InputTextWithHint(
        "##ConsoleInput",
        "Enter Console Command",
        input,
        IM_ARRAYSIZE(input),
        flags,
        &InputCallback,
        this);

// Submit on Enter
    if (submit) {
#if 0
        ExecCommand(input);
#endif
        m_console.SubmitLine(input);
        input[0] = '\0';

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
