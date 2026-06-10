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
    EditorWindow(EditorState& p_editor);

    void drawUI() override;

    virtual const char* windowId() const = 0;

    bool IsOpen() const { return m_state.open; }
    bool IsVisible() const { return m_state.visible; }
    bool IsFocused() const { return m_state.focused; }
    bool IsHovered() const { return m_state.hovered; }

protected:
    virtual void drawUIImpl() = 0;
    void ResetState();
    void UpdateState();

    WindowState m_state;

    int m_flags{ 0 };
};

}  // namespace cave
