#include "LogPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/Color.h"
#include "cave/core/diagnostics/CompositeLogger.h"
#include "cave/core/diagnostics/LogPresentation.h"
#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/IApplication.h"

#include "editor/EditorState.h"
#include "editor/widgets/Image.h"

// @TODO: refactor
#include "ConsolePanel.h"

#include "engine/private/core/os/os.h"

namespace cave {

static constexpr float kLogFilterWidth = 150.0f;

LogPanel::LogPanel(EditorState& editor)
    : EditorWindow(editor)
    , m_level_filter{ LOG_LEVEL_ALL & ~(LOG_LEVEL_TRACE) } {
    m_console = MakeOwner<ConsolePanel>(editor);
}

LogPanel::~LogPanel() = default;

const char* LogPanel::windowId() const {
    return ICON_FA_TERMINAL "  Output Log";
}

static void DrawLog(const LogEvent& log) {
    using ui::IconType;

    ColorCode color = ColorCode::Silver;
    IconType type = IconType::Info;
    switch (log.level) {
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
    std::string formatted = FormatLog(log);
    ImGui::SameLine();
    ImGui::Text("  %s", formatted.c_str());
    if (log.repeat > 1) {
        ImGui::SameLine();
        ImGui::Text(" [ x%u ]", log.repeat);
    }
    ImGui::PopStyleColor();
}

void LogPanel::drawFilter() {
    searchBar();
    ImGui::SameLine();
    verbosityDropDown();
    ImGui::SameLine();
    channelDropDown();
}

void LogPanel::verbosityDropDown() {
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

void LogPanel::channelDropDown() {
    static constexpr const char* s_channels[] = {
#define CAVE_LOG_CHANNEL(x, ...) #x,
        CAVE_LOG_CHANNEL_LIST
#undef CAVE_LOG_CHANNEL
    };

    ImGui::SetNextItemWidth(kLogFilterWidth);

    const char* preview = allChannels() ? "All Channels" : s_channels[(int)m_channel_filter];
    if (ImGui::BeginCombo("##Channel", preview)) {
        if (ImGui::RadioButton("All", allChannels())) {
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

void LogPanel::searchBar() {
    ImGui::Text(ICON_FA_MAGNIFYING_GLASS);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputTextWithHint("##LogSearch", "Search...", m_search_buffer.data(), m_search_buffer.capacity());
}

bool LogPanel::passSearchFilter(const LogEvent& log) const {
    if (m_search_buffer[0] == '\0') {
        return true;
    }

    // @TODO: better string matching
    auto contains = [this](std::string_view p_msg) {
        const bool found = p_msg.find(m_search_buffer.c_str()) != std::string::npos;
        return found;
    };

    return contains(log.message) || contains(log.time_str);
}

void LogPanel::drawLogHistroy() {
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

    CompositeLogger& logger = OS::singleton().logger();
    auto logs = logger.allLogs();
    switch (m_level_filter) {
        case cave::LOG_LEVEL_WARN:
            logs = logger.warningLogs();
            break;
        case cave::LOG_LEVEL_ERROR:
            logs = logger.errorLogs();
            break;
        default:
            break;
    }

    for (const LogEvent& log : logs) {
        if (!(log.level & m_level_filter)) {
            continue;
        }

        if (!allChannels() && log.channel != m_channel_filter) {
            continue;
        }

        if (!passSearchFilter(log)) {
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

void LogPanel::drawUIImpl() {
    CAVE_PROFILE_EVENT();

    drawFilter();
    ImGui::Separator();
    drawLogHistroy();
    ImGui::Separator();
    m_console->DrawConsole();

    // Auto-focus on window apparition
    ImGui::SetItemDefaultFocus();
}

}  // namespace cave
