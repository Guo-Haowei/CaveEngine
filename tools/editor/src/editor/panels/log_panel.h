#pragma once
#include "editor/editor_window.h"

namespace cave {

class LogPanel : public EditorWindow {
public:
    LogPanel(EditorLayer& editor)
        : EditorWindow(editor) {}

    const char* GetTitle() const override {
        return "Console";
    }

    uint32_t GetErrorCount() const { return m_error_count; }
    uint32_t GetWarningCount() const { return m_warning_count; }

protected:
    void UpdateInternal(Scene* scene) override;

private:
    uint32_t m_error_count{ 0 };
    uint32_t m_warning_count{ 0 };
    bool m_auto_scroll{ true };
    bool m_scroll_to_bottom{ false };
    LogLevel m_filter{ LOG_LEVEL_ALL };
};

}  // namespace cave
