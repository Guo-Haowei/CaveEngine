#pragma once
#include "cave/core/containers/FixedString.h"
#include "cave/core/diagnostics/Log.h"

#include "editor/panels/EditorWindow.h"

namespace cave {

class ConsolePanel;

class LogPanel : public EditorWindow {
public:
    explicit LogPanel(EditorState& editor);
    ~LogPanel();

    const char* windowId() const override;

protected:
    void drawUIImpl() override;
    void drawFilter();
    void drawLogHistroy();

    bool allChannels() const {
        return channel_filter_ == LogChannel::Count;
    }

    bool passSearchFilter(const LogEvent& log) const;

    void verbosityDropDown();
    void channelDropDown();
    void searchBar();

    bool auto_scroll_{ true };
    bool scroll_to_bottom_{ false };
    LogLevel level_filter_{ LOG_LEVEL_ALL };
    LogChannel channel_filter_{ LogChannel::Count };
    FixedString<128> search_buffer_;

    // @TODO: make it a standalone window
    std::unique_ptr<ConsolePanel> console_;
};

}  // namespace cave
