#pragma once
#include "engine/private/core/logging/Logger.h"

#include "editor/panels/EditorWindow.h"

namespace cave {

class LogPanel : public EditorWindow {
public:
    LogPanel(EditorState& editor)
        : EditorWindow(editor) {}

    const char* GetWindowId() const override {
        return "Log";
    }

protected:
    void DrawUIImpl() override;

    bool m_auto_scroll{ true };
    bool m_scroll_to_bottom{ false };
    LogLevel m_filter{ LOG_LEVEL_ALL };
};

}  // namespace cave
