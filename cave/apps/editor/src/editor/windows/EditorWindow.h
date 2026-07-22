#pragma once
#include "editor/IEditorItem.h"

namespace cave {

struct WindowState {
    bool open{ false };
    bool visible{ false };
    bool hovered{ false };
    bool focused{ false };
};

class EditorWindow : public IEditorItem {
public:
    EditorWindow(EditorState& editor);

    void drawUI() override;

    virtual const char* windowId() const = 0;

    bool isOpen() const { return m_window_state.open; }
    bool isVisible() const { return m_window_state.visible; }
    bool isFocused() const { return m_window_state.focused; }
    bool isHovered() const { return m_window_state.hovered; }

    const WindowState& windowState() const { return m_window_state; }

protected:
    virtual void drawUIImpl() = 0;
    void resetState();
    void updateState();
    int m_window_flags{ 0 };

private:
    WindowState m_window_state;
};

}  // namespace cave
