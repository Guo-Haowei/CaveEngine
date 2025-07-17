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

protected:
    void UpdateInternal(Scene* scene) override;

private:
    bool m_autoScroll{ true };
    bool m_scrollToBottom{ false };
    LogLevel m_filter{ LOG_LEVEL_ALL };
};

}  // namespace cave
