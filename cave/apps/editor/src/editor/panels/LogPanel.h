#pragma once
#include "cave/core/diagnostics/Log.h"

#include "editor/panels/EditorWindow.h"

struct ImGuiInputTextCallbackData;

namespace cave {

class Console;

class LogPanel : public EditorWindow {
    static constexpr int kCmdBufferSize = 512;

public:
    explicit LogPanel(EditorState& p_editor);

    const char* GetWindowId() const override {
        return "Output Log";
    }

protected:
    void DrawUIImpl() override;
    void DrawFilter();
    void DrawLogHistroy();
    void DrawConsole();

    void VerbosityDropDown();
    void ChannelDropDown();
    bool AllChannels() const { return m_channel_filter == LogChannel::Count; }

    static int InputCallback(ImGuiInputTextCallbackData* p_data);

    Console& m_console;
    bool m_auto_scroll{ true };
    bool m_scroll_to_bottom{ false };
    LogLevel m_level_filter{ LOG_LEVEL_ALL };
    LogChannel m_channel_filter{ LogChannel::Count };
    char m_cmd_buffer[kCmdBufferSize]{};

    class AutoCompletion {
    public:
        [[nodiscard]] std::string_view Current() const;
        [[nodiscard]] std::string_view Next();

        [[nodiscard]] bool Empty() const { return m_cmds.empty(); }

        void Clear() {
            m_cmds.clear();
            m_index = 0;
        }

        void Set(std::vector<std::string_view>&& p_cmds) {
            m_cmds = std::move(p_cmds);
            m_index = 0;
        }

    private:
        std::vector<std::string_view> m_cmds;
        size_t m_index{};
    } m_ac;
};

}  // namespace cave
