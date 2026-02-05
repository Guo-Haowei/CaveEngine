#pragma once
#include "engine/private/core/logging/Logger.h"

#include "editor/panels/EditorWindow.h"

// clang-format off
struct ImGuiInputTextCallbackData;
namespace cave::debug { class Console; }
// clang-format on

namespace cave {

class LogPanel : public EditorWindow {
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

    static int InputCallback(ImGuiInputTextCallbackData* p_data);

    debug::Console& m_console;
    bool m_auto_scroll{ true };
    bool m_scroll_to_bottom{ false };
    LogLevel m_filter{ LOG_LEVEL_ALL };
};

}  // namespace cave
