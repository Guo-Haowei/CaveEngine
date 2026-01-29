#pragma once
#include "cave/runtime/core/geom/Rect.h"

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

    void DrawUI() override;

    virtual const char* GetWindowId() const = 0;

    bool IsOpen() const { return m_state.open; }
    bool IsVisible() const { return m_state.visible; }
    bool IsFocused() const { return m_state.focused; }
    bool IsHovered() const { return m_state.hovered; }

protected:
    virtual void DrawUIImpl() = 0;
    void ResetState();
    void UpdateState();

    WindowState m_state;

    RectF m_rect{};

    int m_flags{ 0 };
};

}  // namespace cave
