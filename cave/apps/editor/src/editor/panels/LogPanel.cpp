#include "LogPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/Color.h"
#include "cave/core/diagnostics/LogPresentation.h"
#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/IApplication.h"

#include "editor/EditorState.h"
#include "editor/widgets/Image.h"

// @TODO: refactor
#include "ConsolePanel.h"
#include "cave/core/diagnostics/CompositeLogger.h"

namespace cave {

static constexpr float kLogFilterWidth = 150.0f;

LogPanel::LogPanel(EditorState& editor)
    : EditorWindow(editor) {
    console_ = std::make_unique<ConsolePanel>(editor);
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
        bool all = level_filter_ == LOG_LEVEL_ALL;
        if (ImGui::Checkbox("All", &all)) {
            if (all) {
                level_filter_ = LOG_LEVEL_ALL;
            } else {
                level_filter_ = static_cast<LogLevel>(0);
            }
        }

        ImGui::Separator();

        auto filter_level = [this](LogLevel level, const char* label) {
            bool flag = level_filter_ & level;
            if (ImGui::Checkbox(label, &flag)) {
                if (flag) {
                    level_filter_ |= level;
                } else {
                    level_filter_ &= ~level;
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

    const char* preview = allChannels() ? "All Channels" : s_channels[(int)channel_filter_];
    if (ImGui::BeginCombo("##Channel", preview)) {
        if (ImGui::RadioButton("All", allChannels())) {
            channel_filter_ = LogChannel::Count;
        }

        ImGui::Separator();

        for (uint16_t i = 0; i < std::to_underlying(LogChannel::Count); ++i) {
            const LogChannel channel = static_cast<LogChannel>(i);
            if (ImGui::RadioButton(s_channels[i], channel_filter_ == channel)) {
                channel_filter_ = channel;
            }
        }

        ImGui::EndCombo();
    }
}

void LogPanel::searchBar() {
    ImGui::Text(ICON_FA_MAGNIFYING_GLASS);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputTextWithHint("##LogSearch", "Search...", search_buffer_.data(), search_buffer_.capacity());
}

bool LogPanel::passSearchFilter(const LogEvent& log) const {
    if (search_buffer_[0] == '\0') {
        return true;
    }

    // @TODO: better string matching
    auto contains = [this](std::string_view p_msg) {
        const bool found = p_msg.find(search_buffer_.c_str()) != std::string::npos;
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

    const std::vector<LogEvent>* logs = &CompositeLogger::singleton().GetAllLogs();
    switch (level_filter_) {
        case cave::LOG_LEVEL_WARN:
            logs = &CompositeLogger::singleton().GetWarningLogs();
            break;
        case cave::LOG_LEVEL_ERROR:
            logs = &CompositeLogger::singleton().GetErrorLogs();
            break;
        default:
            break;
    }

    for (const LogEvent& log : (*logs)) {
        if (!(log.level & level_filter_)) {
            continue;
        }

        if (!allChannels() && log.channel != channel_filter_) {
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

    if (scroll_to_bottom_ || (auto_scroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())) {
        ImGui::SetScrollHereY(1.0f);
    }
    scroll_to_bottom_ = false;

    ImGui::PopStyleVar();
    ImGui::EndChild();
}

void LogPanel::drawUIImpl() {
    CAVE_PROFILE_EVENT();

    drawFilter();
    ImGui::Separator();
    drawLogHistroy();
    ImGui::Separator();
    console_->DrawConsole();

    // Auto-focus on window apparition
    ImGui::SetItemDefaultFocus();
}

}  // namespace cave
