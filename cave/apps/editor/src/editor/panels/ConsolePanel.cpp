#include "ConsolePanel.h"

#include "cave/core/string/StringUtils.h"

#include "engine/private/core/diagnostics/console/Console.h"

#include "editor/EditorState.h"

namespace cave {

ConsolePanel::ConsolePanel(EditorState& p_editor)
    : m_console(p_editor.GetApp().Console()) {
}

void ConsolePanel::DrawConsole() {
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

int ConsolePanel::InputCallback(ImGuiInputTextCallbackData* p_data) {
    ConsolePanel* self = reinterpret_cast<ConsolePanel*>(p_data->UserData);
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

std::string_view ConsolePanel::AutoCompletion::Current() const {
    DEV_ASSERT(!m_cmds.empty());
    return m_cmds[m_index];
}

std::string_view ConsolePanel::AutoCompletion::Next() {
    const size_t size = m_cmds.size();
    DEV_ASSERT(size);
    m_index = (m_index + 1) % size;
    return m_cmds[m_index];
}

}  // namespace cave
