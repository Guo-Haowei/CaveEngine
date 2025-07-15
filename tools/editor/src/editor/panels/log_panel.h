#pragma once
#include "editor/editor_window.h"

namespace cave {

class LogPanel : public EditorWindow {
public:
    LogPanel(EditorLayer& editor)
        : EditorWindow("Console", editor) {}

protected:
    void UpdateInternal(Scene* scene) override;

private:
    bool m_autoScroll{ true };
    bool m_scrollToBottom{ false };
    LogLevel m_filter{ LOG_LEVEL_ALL };
};

}  // namespace cave
