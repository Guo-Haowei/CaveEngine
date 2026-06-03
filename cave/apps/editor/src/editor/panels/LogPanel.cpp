#include "LogPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/Color.h"
#include "cave/core/diagnostics/Profiler.h"
#include "cave/core/string/StringUtils.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/diagnostics/console/Console.h"
#include "engine/private/core/diagnostics/log_sink/LogUtils.h"
#include "engine/private/core/diagnostics/log_sink/CompositeLogger.h"

#include "editor/EditorState.h"
#include "editor/widgets/Image.h"

namespace cave {

static constexpr float kLogFilterWidth = 150.0f;

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
    using ui::IconType;

    ColorCode color = ColorCode::Silver;
    IconType type = IconType::Info;
    switch (p_log.level) {
        case LOG_LEVEL_WARN:
            type = IconType::Exclamation;
            color = ColorCode::Yellow;
            break;
        case LOG_LEVEL_ERROR:
        case LOG_LEVEL_FATAL:
            color = ColorCode::Red;
            type = IconType::Exclamation;
            break;
        case LOG_LEVEL_OK:
            color = ColorCode::Palegreen;
            type = IconType::Check;
            break;
        case LOG_LEVEL_INFO:
            color = ColorCode::White;
            break;
        default:
            break;
    }

    Color c = Color::Hex(color);
    ui::ColorIcon(c, type);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(c.r, c.g, c.b, 1.0f));
    std::string log = detail::FormatLog(p_log);
    ImGui::SameLine();
    ImGui::Text("  %s", log.c_str());
    if (p_log.repeat > 1) {
        ImGui::SameLine();
        ImGui::Text(" [ x%u ]", p_log.repeat);
    }
    ImGui::PopStyleColor();
}

void LogPanel::DrawFilter() {
    SearchBar();
    ImGui::SameLine();
    VerbosityDropDown();
    ImGui::SameLine();
    ChannelDropDown();
}

void LogPanel::VerbosityDropDown() {
    ImGui::SetNextItemWidth(kLogFilterWidth);

    if (ImGui::BeginCombo("##Level", "Verbosity")) {
        bool all = m_level_filter == LOG_LEVEL_ALL;
        if (ImGui::Checkbox("All", &all)) {
            if (all) {
                m_level_filter = LOG_LEVEL_ALL;
            } else {
                m_level_filter = static_cast<LogLevel>(0);
            }
        }

        ImGui::Separator();

        auto filter_level = [this](LogLevel level, const char* label) {
            bool flag = m_level_filter & level;
            if (ImGui::Checkbox(label, &flag)) {
                if (flag) {
                    m_level_filter |= level;
                } else {
                    m_level_filter &= ~level;
                }
            }
        };
#define LOG_LEVEL_COLOR(LEVEL, LABEL, ...) filter_level(LEVEL, LABEL);
        LOG_LEVEL_COLOR_LIST
#undef LOG_LEVEL_COLOR

        ImGui::EndCombo();
    }
}

void LogPanel::ChannelDropDown() {
    static constexpr const char* s_channels[] = {
#define CAVE_LOG_CHANNEL(x, ...) #x,
        CAVE_LOG_CHANNEL_LIST
#undef CAVE_LOG_CHANNEL
    };

    ImGui::SetNextItemWidth(kLogFilterWidth);

    const char* preview = AllChannels() ? "All Channels" : s_channels[(int)m_channel_filter];
    if (ImGui::BeginCombo("##Channel", preview)) {
        if (ImGui::RadioButton("All", AllChannels())) {
            m_channel_filter = LogChannel::Count;
        }

        ImGui::Separator();

        for (uint16_t i = 0; i < std::to_underlying(LogChannel::Count); ++i) {
            const LogChannel channel = static_cast<LogChannel>(i);
            if (ImGui::RadioButton(s_channels[i], m_channel_filter == channel)) {
                m_channel_filter = channel;
            }
        }

        ImGui::EndCombo();
    }
}

void LogPanel::SearchBar() {
    ImGui::Text(ICON_FA_MAGNIFYING_GLASS);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputTextWithHint("##LogSearch", "Search...", m_search.data(), m_search.capacity());
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

bool LogPanel::PassSearchFilter(const LogEvent& p_log) const {
    if (m_search[0] == '\0') {
        return true;
    }

    // @TODO: better string matching
    auto contains = [this](std::string_view p_msg) {
        const bool found = p_msg.find(m_search.c_str()) != std::string::npos;
        return found;
    };

    return contains(p_log.message) || contains(p_log.time_str);
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
    switch (m_level_filter) {
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
        if (!(log.level & m_level_filter)) {
            continue;
        }

        if (!AllChannels() && log.channel != m_channel_filter) {
            continue;
        }

        if (!PassSearchFilter(log)) {
            continue;
        }

        ImVec2 text_pos = ImGui::GetCursorScreenPos();
        ImVec2 text_size = ImGui::CalcTextSize(log.message.c_str());
        text_size.x = std::max(text_size.x, window_size.x);
        text_size.y += padding * 3;
        const float rect_w = text_pos.x + text_size.x + 100.f;

        ImGui::GetWindowDrawList()->AddRectFilled(
            text_pos,
            ImVec2(rect_w, text_pos.y + text_size.y),
            colors[color_index]);

        color_index ^= 1;

        ImGui::Dummy(ImVec2(padding, padding));
        DrawLog(log);
        ImGui::Dummy(ImVec2(padding, padding));
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
