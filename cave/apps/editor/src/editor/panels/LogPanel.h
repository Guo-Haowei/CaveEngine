#pragma once
#include "cave/core/containers/FixedString.h"

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
        return m_channel_filter == LogChannel::Count;
    }

    bool passSearchFilter(const LogEvent& log) const;

    void verbosityDropDown();
    void channelDropDown();
    void searchBar();

    LogLevel m_level_filter;
    LogChannel m_channel_filter{ LogChannel::Count };
    FixedString<128> m_search_buffer;

    bool m_auto_scroll{ true };
    bool m_scroll_to_bottom{ false };

    // @TODO: make it a standalone window
    Owner<ConsolePanel> m_console;
};

}  // namespace cave
