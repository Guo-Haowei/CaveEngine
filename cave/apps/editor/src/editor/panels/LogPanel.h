#pragma once
#include "cave/core/containers/FixedString.h"
#include "cave/core/diagnostics/Log.h"

#include "editor/panels/EditorWindow.h"

namespace cave {

class ConsolePanel;

class LogPanel : public EditorWindow {

public:
    explicit LogPanel(EditorState& p_editor);
    ~LogPanel();

    const char* GetWindowId() const override {
        return "Output Log";
    }

protected:
    void DrawUIImpl() override;
    void DrawFilter();
    void DrawLogHistroy();

    bool AllChannels() const { return m_channel_filter == LogChannel::Count; }
    bool PassSearchFilter(const LogEvent& p_log) const;

    void VerbosityDropDown();
    void ChannelDropDown();
    void SearchBar();

    bool m_auto_scroll{ true };
    bool m_scroll_to_bottom{ false };
    LogLevel m_level_filter{ LOG_LEVEL_ALL };
    LogChannel m_channel_filter{ LogChannel::Count };
    FixedString<128> m_search;

    // @TODO: make it a standalone window
    std::unique_ptr<ConsolePanel> m_console;
};

}  // namespace cave
