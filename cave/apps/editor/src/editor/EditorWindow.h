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
    EditorWindow(EditorState& p_editor)
        : IEditorItem(p_editor) {}

    void Update(float p_timestep) override;

    virtual const char* GetTitle() const = 0;

    bool IsOpen() const { return m_state.open; }
    bool IsVisible() const { return m_state.visible; }
    bool IsFocused() const { return m_state.focused; }
    bool IsHovered() const { return m_state.hovered; }

protected:
    virtual void UpdateInternal(float p_timestep) = 0;

    WindowState m_state;

    int m_flags{ 0 };
};

}  // namespace cave
