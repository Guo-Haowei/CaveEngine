#pragma once
#include "editor/windows/EditorWindow.h"

struct ImGuiInputTextCallbackData;

namespace cave {

class Console;

class ConsolePanel {
    static constexpr int kCmdBufferSize = 512;

public:
    ConsolePanel(EditorState& p_editor);

    void DrawConsole();

private:
    // @TODO: move console to a different place
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

private:
    static int InputCallback(ImGuiInputTextCallbackData* p_data);

    Console& m_console;
    char m_cmd_buffer[kCmdBufferSize]{};
};

}  // namespace cave
